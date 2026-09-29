// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QUANTBTC_PQC_ENCODING_H
#define QUANTBTC_PQC_ENCODING_H

#include <pqc/algorithm.h>
#include <pqc/error.h>
#include <pqc/key.h>

#include <script/script.h>
#include <span>
#include <uint256.h>
#include <util/expected.h>

#include <cstdint>
#include <utility>
#include <vector>

namespace pqc {

/** Envelope version for all consensus-visible PQC blobs (§50). Bump only
 * with a fully specified migration; decoders must reject unknown versions. */
static constexpr uint8_t PQC_ENVELOPE_VERSION = 1;

/** pubkey_blob = ver(1) || alg_id(2, LE) || raw_pubkey. Strict: version must
 * match, algorithm must be recognized, raw size must equal algorithm params. */
util::Expected<std::vector<unsigned char>, PQCError> EncodePubKeyBlob(PQCAlgorithm algo, std::span<const unsigned char> raw_pubkey);
util::Expected<std::pair<PQCAlgorithm, std::vector<unsigned char>>, PQCError> DecodePubKeyBlob(std::span<const unsigned char> blob);

/** sig_blob = ver(1) || alg_id(2, LE) || raw_sig. Same strictness. */
util::Expected<std::vector<unsigned char>, PQCError> EncodeSigBlob(PQCAlgorithm algo, std::span<const unsigned char> raw_sig);
util::Expected<std::pair<PQCAlgorithm, std::vector<unsigned char>>, PQCError> DecodeSigBlob(std::span<const unsigned char> blob);

/** P2PQ scriptPubKey: OP_1 <0x23 <ver(1) || alg_id(2, LE) || keyid(32)>>,
 * where keyid = SHA256(raw_pubkey) in full (never truncated). */
CScript BuildP2PQScript(PQCAlgorithm algo, const uint256& keyid);

struct P2PQInfo {
    uint8_t version{PQC_ENVELOPE_VERSION};
    PQCAlgorithm algo{PQCAlgorithm::UNKNOWN};
    uint256 keyid;
};

/** Parse a P2PQ scriptPubKey. Rejects wrong size/opcodes/versions and
 * non-consensus algorithms (TEST_XOR can never parse as consensus-valid). */
util::Expected<P2PQInfo, PQCError> ParseP2PQScript(const CScript& script);

/** Recompute keyid = SHA256(raw_pubkey) and compare against the commitment. */
PQCError VerifyP2PQCommitment(const P2PQInfo& info, std::span<const unsigned char> raw_pubkey);

} // namespace pqc

#endif // QUANTBTC_PQC_ENCODING_H
