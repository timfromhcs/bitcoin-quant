// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <storage/econ.h>

#include <consensus/amount.h>

#include <cstring>

namespace storage {
namespace {
//! Exact 64x64->128 multiply accumulated into four 32-bit limbs (r0..r3,
//! r0 = least significant). All partials < 2^64; carries tracked explicitly.
//! Proven exact: r0+r1*2^32+r2*2^64+r3*2^96 == a*b (see spec §13 proof note).
void Mul128(uint64_t a, uint64_t b, uint64_t limbs[4])
{
    static constexpr uint64_t MASK32 = 0xFFFFFFFFULL;
    const uint64_t a0 = a & MASK32, a1 = a >> 32;
    const uint64_t b0 = b & MASK32, b1 = b >> 32;
    const uint64_t t0 = a0 * b0, t1 = a0 * b1, t2 = a1 * b0, t3 = a1 * b1;
    // value = t0 + (t1+t2)*2^32 + t3*2^64 ; fold t0's high half upward.
    uint64_t s = (t0 >> 32) + t1;
    uint64_t khi = (s < t1); // units of 2^64
    s += t2;
    khi += (s < t2);
    const uint64_t r0 = t0 & MASK32;
    const uint64_t r1 = s & MASK32;
    uint64_t r2 = (s >> 32) + (t3 & MASK32);
    const uint64_t k2 = r2 >> 32;
    r2 &= MASK32;
    const uint64_t r3 = (t3 >> 32) + k2 + khi;
    limbs[0] = r0;
    limbs[1] = r1;
    limbs[2] = r2;
    limbs[3] = r3; // < 2^32 guaranteed: total < 2^128 with exact low limbs.
}

uint16_t ReadLE16(const unsigned char* p)
{
    return static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << 8));
}

uint64_t ReadLE64(const unsigned char* p)
{
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v |= static_cast<uint64_t>(p[i]) << (8 * i);
    return v;
}

void AppendLE16(std::vector<unsigned char>& v, uint16_t x)
{
    v.push_back(static_cast<unsigned char>(x));
    v.push_back(static_cast<unsigned char>(x >> 8));
}

void AppendLE64(std::vector<unsigned char>& v, uint64_t x)
{
    for (int i = 0; i < 8; ++i) v.push_back(static_cast<unsigned char>(x >> (8 * i)));
}

RecordError ValidateRecord(const ProviderRecord& rec)
{
    if (rec.payout_script.empty() || rec.payout_script.size() > MAX_PAYOUT_SCRIPT_SIZE) {
        return RecordError::BAD_SCRIPT_SIZE;
    }
    if (rec.status != ProviderStatus::ACTIVE && rec.status != ProviderStatus::SUSPENDED &&
        rec.status != ProviderStatus::EXITED) {
        return RecordError::BAD_STATUS;
    }
    if (rec.capacity_bytes == 0) return RecordError::BAD_CAPACITY;
    if (rec.price_sat_per_gib_epoch > MAX_STORAGE_PRICE) return RecordError::BAD_PRICE;
    return RecordError::OK;
}
} // namespace

RecordError DecodeRecord(const unsigned char* data, size_t len, ProviderRecord& out)
{
    ProviderRecord tmp;
    static constexpr size_t MIN_LEN = 1 + 32 + 2 + 1 + 8 + 8 + 1 + 8;
    if (len < MIN_LEN) return RecordError::TOO_SHORT;
    if (data[0] != RECORD_VERSION) return RecordError::BAD_VERSION;
    size_t pos = 1;
    std::memcpy(tmp.provider_id.begin(), data + pos, 32);
    pos += 32;
    const uint16_t slen = ReadLE16(data + pos);
    pos += 2;
    if (slen == 0 || slen > MAX_PAYOUT_SCRIPT_SIZE) return RecordError::BAD_SCRIPT_SIZE;
    if (len != pos + slen + 8 + 8 + 1 + 8) return RecordError::BAD_SCRIPT_SIZE;
    tmp.payout_script.assign(data + pos, data + pos + slen);
    pos += slen;
    tmp.capacity_bytes = ReadLE64(data + pos);
    pos += 8;
    tmp.price_sat_per_gib_epoch = ReadLE64(data + pos);
    pos += 8;
    tmp.status = static_cast<ProviderStatus>(data[pos]);
    pos += 1;
    tmp.registered_epoch = ReadLE64(data + pos);
    const RecordError err = ValidateRecord(tmp);
    if (err != RecordError::OK) return err;
    out = std::move(tmp);
    return RecordError::OK;
}

std::vector<unsigned char> EncodeRecord(const ProviderRecord& rec)
{
    std::vector<unsigned char> out;
    out.reserve(52 + rec.payout_script.size());
    out.push_back(RECORD_VERSION);
    out.insert(out.end(), rec.provider_id.begin(), rec.provider_id.end());
    AppendLE16(out, static_cast<uint16_t>(rec.payout_script.size()));
    out.insert(out.end(), rec.payout_script.begin(), rec.payout_script.end());
    AppendLE64(out, rec.capacity_bytes);
    AppendLE64(out, rec.price_sat_per_gib_epoch);
    out.push_back(static_cast<unsigned char>(rec.status));
    AppendLE64(out, rec.registered_epoch);
    return out;
}

bool TransitionOk(ProviderStatus from, ProviderStatus to)
{
    switch (from) {
    case ProviderStatus::ACTIVE:
        return to == ProviderStatus::SUSPENDED || to == ProviderStatus::EXITED;
    case ProviderStatus::SUSPENDED:
        return to == ProviderStatus::ACTIVE || to == ProviderStatus::EXITED;
    case ProviderStatus::EXITED:
        return false;
    }
    return false;
}

RewardResult CalcReward(uint64_t capacity_bytes, uint64_t price_sat_per_gib_epoch,
                        uint64_t verified_bytes, int64_t& amount_out)
{
    if (capacity_bytes == 0 || price_sat_per_gib_epoch > MAX_STORAGE_PRICE ||
        verified_bytes > capacity_bytes) {
        return RewardResult::REWARD_INVALID_INPUT;
    }
    uint64_t limbs[4];
    Mul128(verified_bytes, price_sat_per_gib_epoch, limbs);
    // Divide by 2^30 (GiB): shift right 30 across limbs, exact.
    // Result limb X = product bits [32X+30, 32X+62): the top SHIFT bits of
    // limb X move down, and the low SHIFT bits of limb X+1 enter at offset
    // (32-SHIFT). Telescopes exactly to (product >> SHIFT).
    static constexpr unsigned SHIFT = 30;
    static constexpr uint64_t LOWMASK = (1ULL << SHIFT) - 1; // low SHIFT bits
    static constexpr unsigned IN_OFF = 32 - SHIFT;
    const uint64_t nr0 = (limbs[0] >> SHIFT) | ((limbs[1] & LOWMASK) << IN_OFF);
    const uint64_t nr1 = (limbs[1] >> SHIFT) | ((limbs[2] & LOWMASK) << IN_OFF);
    const uint64_t nr2 = (limbs[2] >> SHIFT) | ((limbs[3] & LOWMASK) << IN_OFF);
    const uint64_t nr3 = limbs[3] >> SHIFT;
    if (nr2 != 0 || nr3 != 0) return RewardResult::REWARD_OVERFLOW;
    const uint64_t amount = nr0 | (nr1 << 32);
    if (amount > static_cast<uint64_t>(MAX_MONEY)) return RewardResult::REWARD_OVERFLOW;
    amount_out = static_cast<int64_t>(amount);
    return RewardResult::REWARD_OK;
}
} // namespace storage
