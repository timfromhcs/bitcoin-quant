// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/merkle.h>
#include <storage/proof.h>
#include <test/storage_test_vectors.h>
#include <uint256.h>
#include <util/strencodings.h>

#include <boost/test/unit_test.hpp>

#include <cstring>
#include <vector>

//! Raw-bytes hex (Python ref byte order) -> uint256 internal bytes.
//! NOTE: NOT uint256S/SetHex (those reverse for display convention).
static uint256 RawHex(const std::string& hex)
{
    std::vector<unsigned char> b{ParseHex(hex)};
    BOOST_REQUIRE_EQUAL(b.size(), 32U);
    uint256 h;
    std::memcpy(h.begin(), b.data(), 32);
    return h;
}

static std::vector<unsigned char> RawBytes(const std::string& hex)
{
    return std::vector<unsigned char>{ParseHex(hex)};
}

BOOST_AUTO_TEST_SUITE(storage_tests)

//! Differential: C++ matches the Python reference on every vector.
BOOST_AUTO_TEST_CASE(differential_vectors)
{
    BOOST_REQUIRE(!STORAGE_TEST_VECTORS.empty());
    for (const StorageTestVector& v : STORAGE_TEST_VECTORS) {
        const std::vector<unsigned char> content = RawBytes(v.content_hex);
        // Rebuild leaves the way a storer would: fixed-size chunks.
        std::vector<uint256> leaves;
        for (size_t off = 0; off < content.size(); off += storage::CHUNK_SIZE) {
            size_t n = std::min<size_t>(storage::CHUNK_SIZE, content.size() - off);
            leaves.push_back(storage::ChunkHash(content.data() + off, n));
        }
        BOOST_REQUIRE(!leaves.empty());
        BOOST_CHECK(storage::MerkleRoot(leaves) == RawHex(v.root_hex));

        auto idx = storage::DeriveIndices(RawHex(v.root_hex), RawHex(v.provider_hex),
                                          v.epoch, (uint32_t)leaves.size());
        BOOST_REQUIRE_EQUAL(idx.size(), v.indices.size());
        for (size_t i = 0; i < idx.size(); ++i) BOOST_CHECK_EQUAL(idx[i], v.indices[i]);

        // Reference proof verifies, envelope round-trips byte-identical.
        storage::Proof pr;
        pr.index = v.proof_index;
        pr.chunk = RawBytes(v.chunk_hex);
        for (const std::string& s : v.path_hex) pr.path.push_back(RawHex(s));
        BOOST_CHECK(storage::Verify(RawHex(v.root_hex), pr) == storage::VerifyResult::OK);
        BOOST_CHECK(storage::EncodeProof(pr) == RawBytes(v.envelope_hex));
        storage::Proof dec;
        BOOST_REQUIRE(storage::DecodeProof(RawBytes(v.envelope_hex).data(),
                                           RawBytes(v.envelope_hex).size(), dec));
        BOOST_CHECK(dec.index == pr.index);
        BOOST_CHECK(dec.chunk == pr.chunk);
        BOOST_CHECK(dec.path == pr.path);
    }
}

//! Single-source (§157): storage Merkle == consensus Merkle on same leaves.
BOOST_AUTO_TEST_CASE(merkle_matches_consensus)
{
    std::vector<uint256> leaves;
    for (int i = 0; i < 5; ++i) {
        unsigned char b[4] = {(unsigned char)i, 0xAA, 0xBB, 0xCC};
        leaves.push_back(storage::ChunkHash(b, sizeof(b)));
    }
    bool mutated = false;
    BOOST_CHECK(storage::MerkleRoot(leaves) == ComputeMerkleRoot(std::move(leaves), &mutated));
    BOOST_CHECK(!mutated);
}

//! Adversarial negatives: every tamper class must fail distinctly.
BOOST_AUTO_TEST_CASE(verify_negatives)
{
    const StorageTestVector& v = STORAGE_TEST_VECTORS.back();
    const uint256 root = RawHex(v.root_hex);
    storage::Proof pr;
    pr.index = v.proof_index;
    pr.chunk = RawBytes(v.chunk_hex);
    for (const std::string& s : v.path_hex) pr.path.push_back(RawHex(s));

    storage::Proof bad = pr;
    bad.chunk[0] ^= 1;
    BOOST_CHECK(storage::Verify(root, bad) == storage::VerifyResult::ROOT_MISMATCH);
    bad = pr;
    if (!bad.path.empty()) {
        bad.path[0].begin()[0] ^= 1;
        BOOST_CHECK(storage::Verify(root, bad) == storage::VerifyResult::ROOT_MISMATCH);
    }
    BOOST_CHECK(storage::Verify(uint256{}, pr) == storage::VerifyResult::ROOT_MISMATCH);
    bad = pr;
    bad.chunk.clear();
    BOOST_CHECK(storage::Verify(root, bad) == storage::VerifyResult::BAD_CHUNK_SIZE);
    bad = pr;
    bad.path.pop_back();
    if (pr.path.size() > 0) {
        auto r = storage::Verify(root, bad);
        BOOST_CHECK(r == storage::VerifyResult::PATH_LENGTH_MISMATCH ||
                    r == storage::VerifyResult::ROOT_MISMATCH);
    }
}

//! Envelope parser: strict rejects, version gate, determinism.
BOOST_AUTO_TEST_CASE(envelope_strict)
{
    const StorageTestVector& v = STORAGE_TEST_VECTORS.front();
    storage::Proof pr;
    pr.index = v.proof_index;
    pr.chunk = RawBytes(v.chunk_hex);
    for (const std::string& s : v.path_hex) pr.path.push_back(RawHex(s));
    const std::vector<unsigned char> enc = storage::EncodeProof(pr);
    storage::Proof dec;
    BOOST_REQUIRE(storage::DecodeProof(enc.data(), enc.size(), dec));
    // Trailing byte, truncation, version bump, empty: all rejected.
    std::vector<unsigned char> with_trail = enc;
    with_trail.push_back(0);
    BOOST_CHECK(!storage::DecodeProof(with_trail.data(), with_trail.size(), dec));
    BOOST_CHECK(!storage::DecodeProof(enc.data(), enc.size() - 1, dec));
    std::vector<unsigned char> badver = enc;
    badver[0] = 2;
    BOOST_CHECK(!storage::DecodeProof(badver.data(), badver.size(), dec));
    BOOST_CHECK(!storage::DecodeProof(nullptr, 0, dec));
    // Determinism: same inputs, same outputs.
    BOOST_CHECK(storage::EncodeProof(pr) == enc);
    BOOST_CHECK(storage::DeriveIndices(RawHex(v.root_hex), RawHex(v.provider_hex),
                                       v.epoch, 1) ==
                storage::DeriveIndices(RawHex(v.root_hex), RawHex(v.provider_hex),
                                       v.epoch, 1));
}

BOOST_AUTO_TEST_SUITE_END()
