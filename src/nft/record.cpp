// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <nft/record.h>

#include <crypto/sha256.h>

#include <cstring>

namespace nft {
namespace {
void DoubleSHA256(const unsigned char* data, size_t len, unsigned char out[32])
{
    unsigned char buf[CSHA256::OUTPUT_SIZE];
    CSHA256().Write(data, len).Finalize(buf);
    CSHA256().Write(buf, sizeof(buf)).Finalize(out);
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

uint64_t ReadBE64(const unsigned char* p)
{
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v = (v << 8) | p[i];
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

void AppendBE64(std::vector<unsigned char>& v, uint64_t x)
{
    for (int i = 7; i >= 0; --i) v.push_back(static_cast<unsigned char>(x >> (8 * i)));
}
} // namespace

uint256 NftId(const uint256& collection_id, uint64_t serial)
{
    std::vector<unsigned char> pre;
    pre.reserve(40);
    pre.insert(pre.end(), collection_id.begin(), collection_id.end());
    AppendBE64(pre, serial);
    uint256 id;
    DoubleSHA256(pre.data(), pre.size(), id.begin());
    return id;
}

MintError DecodeMint(const unsigned char* data, size_t len, MintRecord& out)
{
    MintRecord tmp;
    static constexpr size_t MIN_LEN = 1 + 32 + 8 + 32 + 32 + 2 + 1 + 8;
    if (len < MIN_LEN) return MintError::TooShort;
    if (data[0] != MINT_RECORD_VERSION) return MintError::BadVersion;
    size_t pos = 1;
    std::memcpy(tmp.collection_id.begin(), data + pos, 32);
    pos += 32;
    tmp.serial = ReadBE64(data + pos);
    pos += 8;
    std::memcpy(tmp.content_root.begin(), data + pos, 32);
    pos += 32;
    std::memcpy(tmp.metadata_hash.begin(), data + pos, 32);
    pos += 32;
    const uint16_t slen = ReadLE16(data + pos);
    pos += 2;
    if (slen == 0 || slen > MAX_OWNER_SCRIPT_SIZE) return MintError::BadScriptSize;
    if (len != pos + slen + 8) return MintError::BadScriptSize;
    tmp.owner_script.assign(data + pos, data + pos + slen);
    pos += slen;
    tmp.minted_epoch = ReadLE64(data + pos);
    out = std::move(tmp);
    return MintError::Ok;
}

std::vector<unsigned char> EncodeMint(const MintRecord& rec)
{
    std::vector<unsigned char> out;
    out.reserve(114 + rec.owner_script.size());
    out.push_back(MINT_RECORD_VERSION);
    out.insert(out.end(), rec.collection_id.begin(), rec.collection_id.end());
    AppendBE64(out, rec.serial);
    out.insert(out.end(), rec.content_root.begin(), rec.content_root.end());
    out.insert(out.end(), rec.metadata_hash.begin(), rec.metadata_hash.end());
    AppendLE16(out, static_cast<uint16_t>(rec.owner_script.size()));
    out.insert(out.end(), rec.owner_script.begin(), rec.owner_script.end());
    AppendLE64(out, rec.minted_epoch);
    return out;
}
} // namespace nft
