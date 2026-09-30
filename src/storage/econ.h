// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_STORAGE_ECON_H
#define BITCOIN_STORAGE_ECON_H

#include <uint256.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace storage {
//! Storage economy core (QuantBTC protocol v1, spec §13).
//! Pure math + record codec. No consensus, no I/O, no globals — same DAG
//! rule as proof.h (links bitcoin_crypto only; MAX_MONEY is header-only).
static constexpr unsigned char RECORD_VERSION = 1;
static constexpr size_t MAX_PAYOUT_SCRIPT_SIZE = 10000;
//! Price ceiling (satoshis per GiB per epoch): total-supply order bound.
static constexpr uint64_t MAX_STORAGE_PRICE = 21000000ULL * 100000000ULL;
static constexpr uint64_t GIB_BYTES = 1ULL << 30;

enum class ProviderStatus : uint8_t {
    ACTIVE = 1,
    SUSPENDED = 2,
    EXITED = 3,
};

struct ProviderRecord {
    uint256 provider_id;
    std::vector<unsigned char> payout_script;
    uint64_t capacity_bytes{0};
    uint64_t price_sat_per_gib_epoch{0};
    ProviderStatus status{ProviderStatus::ACTIVE};
    uint64_t registered_epoch{0};
};

enum class RecordError {
    OK,
    TOO_SHORT,
    BAD_VERSION,
    BAD_SCRIPT_SIZE,
    BAD_STATUS,
    BAD_CAPACITY,
    BAD_PRICE,
};

//! Strict decode: exact length match, version gate, structural validation.
//! (Registry uniqueness, operator binding, full script validity are
//! follow-up layers — see spec §13 gaps.)
RecordError DecodeRecord(const unsigned char* data, size_t len, ProviderRecord& out);

//! Encode (precondition: structurally valid record; decode validates).
std::vector<unsigned char> EncodeRecord(const ProviderRecord& rec);

//! Status machine: ACTIVE->{SUSPENDED,EXITED}, SUSPENDED->{ACTIVE,EXITED},
//! EXITED terminal.
bool TransitionOk(ProviderStatus from, ProviderStatus to);

enum class RewardResult {
    REWARD_OK,
    REWARD_OVERFLOW, //!< exact amount exceeds MAX_MONEY
    REWARD_INVALID_INPUT, //!< bad bounds (zero capacity, price cap, verified > capacity, negative impossible by type)
};

//! amount = floor(verified_bytes * price / 2^30), exact 128-bit intermediate
//! via portable 32-bit limbs (no __int128: MSVC). Single-epoch accounting:
//! verified_bytes must not exceed capacity_bytes.
RewardResult CalcReward(uint64_t capacity_bytes, uint64_t price_sat_per_gib_epoch,
                        uint64_t verified_bytes, int64_t& amount_out);
} // namespace storage

#endif // BITCOIN_STORAGE_ECON_H
