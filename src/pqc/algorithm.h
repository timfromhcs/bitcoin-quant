// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QUANTBTC_PQC_ALGORITHM_H
#define QUANTBTC_PQC_ALGORITHM_H

#include <cstddef>
#include <cstdint>
#include <optional>

namespace pqc {

/** Consensus-visible post-quantum algorithm identifiers (§50).
 * Values are frozen for protocol v1; new algorithms require a version bump. */
enum class PQCAlgorithm : uint16_t {
    UNKNOWN = 0x0000,          //!< never valid
    ML_DSA_65 = 0x0001,        //!< ML-DSA-65, FIPS 204 (primary candidate)
    SLH_DSA_SHA2_128S = 0x0002,//!< SLH-DSA-SHA2-128s, FIPS 205 (reserved, no backend yet)
    TEST_XOR = 0xFFFF,         //!< deterministic test stand-in; NEVER consensus-valid
};

struct PQCAlgorithmParams {
    const char* name;
    const char* standard;
    size_t pubkey_size;
    size_t privkey_size;
    size_t sig_size;
    int nist_level;
};

/** Parameters for a known algorithm id; nullopt for UNKNOWN/unlisted. */
std::optional<PQCAlgorithmParams> GetAlgorithmParams(PQCAlgorithm algo);

/** True only for algorithms admissible in consensus (excludes UNKNOWN and TEST_XOR). */
inline bool IsConsensusAlgorithm(PQCAlgorithm algo)
{
    return algo == PQCAlgorithm::ML_DSA_65 || algo == PQCAlgorithm::SLH_DSA_SHA2_128S;
}

} // namespace pqc

#endif // QUANTBTC_PQC_ALGORITHM_H
