// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <storage/proof.h>

#include <crypto/sha256.h>

#include <cassert>
#include <cstring>

namespace storage {
namespace {
void DoubleSHA256(const unsigned char* data, size_t len, unsigned char out[32])
{
    unsigned char buf[CSHA256::OUTPUT_SIZE];
    CSHA256().Write(data, len).Finalize(buf);
    CSHA256().Write(buf, sizeof(buf)).Finalize(out);
}

uint256 DoubleSHA256Vec(const std::vector<unsigned char>& v)
{
    uint256 h;
    DoubleSHA256(v.data(), v.size(), h.begin());
    return h;
}

void AppendBE32(std::vector<unsigned char>& v, uint32_t x)
{
    v.push_back(static_cast<unsigned char>(x >> 24));
    v.push_back(static_cast<unsigned char>(x >> 16));
    v.push_back(static_cast<unsigned char>(x >> 8));
    v.push_back(static_cast<unsigned char>(x));
}

void AppendBE64(std::vector<unsigned char>& v, uint64_t x)
{
    for (int i = 7; i >= 0; --i) v.push_back(static_cast<unsigned char>(x >> (8 * i)));
}

//! Interpret 32 big-endian bytes as an integer mod n (exact 64-bit math:
//! acc < 2^32 at all times, so acc*256+byte never overflows).
uint32_t ModN(const unsigned char bytes[32], uint32_t n)
{
    uint64_t acc = 0;
    for (int i = 0; i < 32; ++i) acc = (acc * 256 + bytes[i]) % n;
    return static_cast<uint32_t>(acc);
}
} // namespace

uint256 ChunkHash(const unsigned char* data, size_t len)
{
    assert(data != nullptr || len == 0);
    uint256 h;
    DoubleSHA256(data, len, h.begin());
    return h;
}

uint256 MerkleRoot(const std::vector<uint256>& leaves)
{
    assert(!leaves.empty());
    std::vector<uint256> level = leaves;
    std::vector<unsigned char> buf;
    buf.reserve(64);
    while (level.size() > 1) {
        if (level.size() % 2 == 1) level.push_back(level.back());
        std::vector<uint256> next;
        next.reserve(level.size() / 2);
        for (size_t i = 0; i < level.size(); i += 2) {
            buf.clear();
            buf.insert(buf.end(), level[i].begin(), level[i].end());
            buf.insert(buf.end(), level[i + 1].begin(), level[i + 1].end());
            next.push_back(DoubleSHA256Vec(buf));
        }
        level = std::move(next);
    }
    return level[0];
}

std::vector<uint32_t> DeriveIndices(const uint256& root, const uint256& provider_id,
                                    uint64_t epoch, uint32_t n_leaves, uint32_t num_samples)
{
    assert(n_leaves > 0);
    std::vector<unsigned char> pre;
    pre.reserve(72);
    pre.insert(pre.end(), root.begin(), root.end());
    pre.insert(pre.end(), provider_id.begin(), provider_id.end());
    AppendBE64(pre, epoch);
    uint256 seed = DoubleSHA256Vec(pre);
    std::vector<uint32_t> out;
    out.reserve(num_samples);
    for (uint32_t i = 0; i < num_samples; ++i) {
        std::vector<unsigned char> q(seed.begin(), seed.end());
        AppendBE32(q, i);
        unsigned char h[32];
        DoubleSHA256(q.data(), q.size(), h);
        out.push_back(ModN(h, n_leaves));
    }
    return out;
}

VerifyResult Verify(const uint256& root, const Proof& proof)
{
    if (proof.chunk.empty() || proof.chunk.size() > CHUNK_SIZE) return VerifyResult::BAD_CHUNK_SIZE;
    if (proof.path.size() > MAX_PATH_DEPTH) return VerifyResult::BAD_PATH;
    uint256 h = ChunkHash(proof.chunk.data(), proof.chunk.size());
    uint32_t idx = proof.index;
    std::vector<unsigned char> buf;
    buf.reserve(64);
    for (const uint256& sib : proof.path) {
        buf.clear();
        if (idx % 2 == 0) {
            buf.insert(buf.end(), h.begin(), h.end());
            buf.insert(buf.end(), sib.begin(), sib.end());
        } else {
            buf.insert(buf.end(), sib.begin(), sib.end());
            buf.insert(buf.end(), h.begin(), h.end());
        }
        h = DoubleSHA256Vec(buf);
        idx /= 2;
    }
    if (idx != 0) return VerifyResult::PATH_LENGTH_MISMATCH;
    if (!(h == root)) return VerifyResult::ROOT_MISMATCH;
    return VerifyResult::OK;
}

std::vector<unsigned char> EncodeProof(const Proof& proof)
{
    std::vector<unsigned char> out;
    out.reserve(13 + proof.path.size() * 32 + proof.chunk.size());
    out.push_back(ENVELOPE_VERSION);
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<unsigned char>(proof.index >> (8 * i)));
    uint32_t n = static_cast<uint32_t>(proof.path.size());
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<unsigned char>(n >> (8 * i)));
    for (const uint256& sib : proof.path) out.insert(out.end(), sib.begin(), sib.end());
    uint32_t c = static_cast<uint32_t>(proof.chunk.size());
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<unsigned char>(c >> (8 * i)));
    out.insert(out.end(), proof.chunk.begin(), proof.chunk.end());
    return out;
}

bool DecodeProof(const unsigned char* data, size_t len, Proof& out)
{
    Proof tmp;
    if (len < 13 || data[0] != ENVELOPE_VERSION) return false;
    uint32_t idx = 0, npath = 0;
    for (int i = 0; i < 4; ++i) idx |= static_cast<uint32_t>(data[1 + i]) << (8 * i);
    for (int i = 0; i < 4; ++i) npath |= static_cast<uint32_t>(data[5 + i]) << (8 * i);
    if (npath > MAX_PATH_DEPTH) return false;
    size_t pos = 9;
    if (len < pos + static_cast<size_t>(npath) * 32 + 4) return false;
    tmp.index = idx;
    tmp.path.reserve(npath);
    for (uint32_t i = 0; i < npath; ++i) {
        uint256 sib;
        std::memcpy(sib.begin(), data + pos, 32);
        tmp.path.push_back(sib);
        pos += 32;
    }
    uint32_t clen = 0;
    for (int i = 0; i < 4; ++i) clen |= static_cast<uint32_t>(data[pos + i]) << (8 * i);
    pos += 4;
    if (clen == 0 || clen > CHUNK_SIZE || len != pos + clen) return false;
    tmp.chunk.assign(data + pos, data + pos + clen);
    out = std::move(tmp);
    return true;
}
} // namespace storage
