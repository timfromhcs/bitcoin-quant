// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_STORAGE_PROOF_H
#define BITCOIN_STORAGE_PROOF_H

#include <uint256.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace storage {
//! Storage proof-of-retrievability core (QuantBTC protocol v1, spec §12).
//!
//! PDP-lite with Merkle spot-checks over SHA256d-chunked content:
//!   leaf(i) = SHA256d(chunk_i), binary tree with Bitcoin duplicate-odd
//!   semantics, challenge indices derived from protocol entropy
//!   (content root || provider id || epoch) so the provider cannot grind.
//!
//! Pure functions only: no consensus, no I/O, no globals. Safe to link
//! without bitcoin_consensus (the future consensus hook will call inward).
static constexpr uint32_t CHUNK_SIZE = 65536;
static constexpr uint32_t CHALLENGE_SAMPLES = 16;
static constexpr unsigned char ENVELOPE_VERSION = 1;
//! Cap on Merkle path depth (bounds proof size; 2^64 leaves max).
static constexpr size_t MAX_PATH_DEPTH = 64;

//! SHA256d of one content chunk.
uint256 ChunkHash(const unsigned char* data, size_t len);

//! Merkle root over leaf hashes (duplicates the odd node, like consensus).
//! Precondition: !leaves.empty().
uint256 MerkleRoot(const std::vector<uint256>& leaves);

//! Deterministic challenge indices in [0, n_leaves). Pure function of
//! (root, provider_id, epoch); identical inputs give identical indices.
std::vector<uint32_t> DeriveIndices(const uint256& root, const uint256& provider_id,
                                    uint64_t epoch, uint32_t n_leaves,
                                    uint32_t num_samples = CHALLENGE_SAMPLES);

struct Proof {
    uint32_t index{0};
    std::vector<unsigned char> chunk;
    //! Sibling hashes bottom-up (leaf level first).
    std::vector<uint256> path;
};

enum class VerifyResult {
    OK,
    MALFORMED,
    BAD_CHUNK_SIZE,
    BAD_PATH,
    PATH_LENGTH_MISMATCH,
    ROOT_MISMATCH,
};

//! Fold the proof's chunk through its path and compare to root.
VerifyResult Verify(const uint256& root, const Proof& proof);

//! Strict versioned envelope: ver(1) || idx LE32 || pathlen LE32 ||
//! siblings(32B) || chunklen LE32 || chunk.
std::vector<unsigned char> EncodeProof(const Proof& proof);

//! Strict decode: exact length match, version check, size bounds.
//! Returns false on any deviation (no partial reads).
bool DecodeProof(const unsigned char* data, size_t len, Proof& out);
} // namespace storage

#endif // BITCOIN_STORAGE_PROOF_H
