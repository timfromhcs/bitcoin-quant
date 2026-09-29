// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pqc/encoding.h>

#include <crypto/sha256.h>
#include <hash.h>

namespace pqc {

namespace {
util::Expected<std::vector<unsigned char>, PQCError> EncodeBlob(PQCAlgorithm algo, std::span<const unsigned char> raw, size_t expect)
{
    const auto params{GetAlgorithmParams(algo)};
    if (!params) return util::Unexpected(PQCError::UNKNOWN_ALGORITHM);
    if (raw.size() != expect) return util::Unexpected(PQCError::BAD_SIZE);
    std::vector<unsigned char> out;
    out.reserve(3 + raw.size());
    out.push_back(PQC_ENVELOPE_VERSION);
    const auto id{static_cast<uint16_t>(algo)};
    out.push_back(id & 0xff);
    out.push_back((id >> 8) & 0xff);
    out.insert(out.end(), raw.begin(), raw.end());
    return out;
}

util::Expected<std::pair<PQCAlgorithm, std::vector<unsigned char>>, PQCError> DecodeBlob(std::span<const unsigned char> blob, size_t expect)
{
    if (blob.size() < 3) return util::Unexpected(PQCError::BAD_ENCODING);
    if (blob[0] != PQC_ENVELOPE_VERSION) return util::Unexpected(PQCError::BAD_VERSION);
    const auto algo{static_cast<PQCAlgorithm>(uint16_t(blob[1]) | (uint16_t(blob[2]) << 8))};
    const auto params{GetAlgorithmParams(algo)};
    if (!params) return util::Unexpected(PQCError::UNKNOWN_ALGORITHM);
    if (blob.size() != 3 + expect) return util::Unexpected(PQCError::BAD_SIZE);
    return std::make_pair(algo, std::vector<unsigned char>(blob.begin() + 3, blob.end()));
}
} // namespace

util::Expected<std::vector<unsigned char>, PQCError> EncodePubKeyBlob(PQCAlgorithm algo, std::span<const unsigned char> raw_pubkey)
{
    const auto params{GetAlgorithmParams(algo)};
    if (!params) return util::Unexpected(PQCError::UNKNOWN_ALGORITHM);
    return EncodeBlob(algo, raw_pubkey, params->pubkey_size);
}

util::Expected<std::pair<PQCAlgorithm, std::vector<unsigned char>>, PQCError> DecodePubKeyBlob(std::span<const unsigned char> blob)
{
    // Size depends on the algorithm tag; decode generically then enforce.
    if (blob.size() < 3) return util::Unexpected(PQCError::BAD_ENCODING);
    if (blob[0] != PQC_ENVELOPE_VERSION) return util::Unexpected(PQCError::BAD_VERSION);
    const auto algo{static_cast<PQCAlgorithm>(uint16_t(blob[1]) | (uint16_t(blob[2]) << 8))};
    const auto params{GetAlgorithmParams(algo)};
    if (!params) return util::Unexpected(PQCError::UNKNOWN_ALGORITHM);
    return DecodeBlob(blob, params->pubkey_size);
}

util::Expected<std::vector<unsigned char>, PQCError> EncodeSigBlob(PQCAlgorithm algo, std::span<const unsigned char> raw_sig)
{
    const auto params{GetAlgorithmParams(algo)};
    if (!params) return util::Unexpected(PQCError::UNKNOWN_ALGORITHM);
    return EncodeBlob(algo, raw_sig, params->sig_size);
}

util::Expected<std::pair<PQCAlgorithm, std::vector<unsigned char>>, PQCError> DecodeSigBlob(std::span<const unsigned char> blob)
{
    if (blob.size() < 3) return util::Unexpected(PQCError::BAD_ENCODING);
    if (blob[0] != PQC_ENVELOPE_VERSION) return util::Unexpected(PQCError::BAD_VERSION);
    const auto algo{static_cast<PQCAlgorithm>(uint16_t(blob[1]) | (uint16_t(blob[2]) << 8))};
    const auto params{GetAlgorithmParams(algo)};
    if (!params) return util::Unexpected(PQCError::UNKNOWN_ALGORITHM);
    return DecodeBlob(blob, params->sig_size);
}

CScript BuildP2PQScript(PQCAlgorithm algo, const uint256& keyid)
{
    // OP_1 <35: ver || alg_id LE || keyid>. Caller must pass a consensus
    // algorithm; TEST_XOR here would build a script no validator accepts.
    CScript script;
    script << OP_1;
    std::vector<unsigned char> data;
    data.reserve(35);
    data.push_back(PQC_ENVELOPE_VERSION);
    const auto id{static_cast<uint16_t>(algo)};
    data.push_back(id & 0xff);
    data.push_back((id >> 8) & 0xff);
    data.insert(data.end(), keyid.begin(), keyid.end());
    script << data;
    return script;
}

util::Expected<P2PQInfo, PQCError> ParseP2PQScript(const CScript& script)
{
    // Fixed 37-byte shape: 0x51 0x23 <35 bytes>.
    if (script.size() != 37 || script[0] != OP_1 || script[1] != 35) {
        return util::Unexpected(PQCError::BAD_ENCODING);
    }
    if (script[2] != PQC_ENVELOPE_VERSION) return util::Unexpected(PQCError::BAD_VERSION);
    const auto algo{static_cast<PQCAlgorithm>(uint16_t(script[3]) | (uint16_t(script[4]) << 8))};
    if (!GetAlgorithmParams(algo)) return util::Unexpected(PQCError::UNKNOWN_ALGORITHM);
    if (!IsConsensusAlgorithm(algo)) {
        // TEST_XOR (and any non-consensus id) can never validate.
        return util::Unexpected(PQCError::TEST_ONLY);
    }
    P2PQInfo info;
    info.version = script[2];
    info.algo = algo;
    std::copy(script.begin() + 5, script.begin() + 37, info.keyid.begin());
    return info;
}

PQCError VerifyP2PQCommitment(const P2PQInfo& info, std::span<const unsigned char> raw_pubkey)
{
    const auto params{GetAlgorithmParams(info.algo)};
    if (!params) return PQCError::UNKNOWN_ALGORITHM;
    if (raw_pubkey.size() != params->pubkey_size) return PQCError::BAD_SIZE;
    uint256 recomputed;
    CSHA256().Write(raw_pubkey.data(), raw_pubkey.size()).Finalize(recomputed.begin());
    if (recomputed != info.keyid) return PQCError::WRONG_KEY;
    return PQCError::OK;
}

} // namespace pqc
