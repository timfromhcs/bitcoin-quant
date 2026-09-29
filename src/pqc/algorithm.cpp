// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pqc/algorithm.h>

namespace pqc {

std::optional<PQCAlgorithmParams> GetAlgorithmParams(PQCAlgorithm algo)
{
    // Sizes from the normative standards (FIPS 204 §5/7, FIPS 205 §5/9).
    // Cross-checked against the production backend at test time.
    switch (algo) {
    case PQCAlgorithm::ML_DSA_65:
        return PQCAlgorithmParams{"ML-DSA-65", "FIPS 204", 1952, 4032, 3309, 3};
    case PQCAlgorithm::SLH_DSA_SHA2_128S:
        // Reserved: format-level parameters known, no backend registered yet.
        return PQCAlgorithmParams{"SLH-DSA-SHA2-128s", "FIPS 205", 32, 64, 7856, 1};
    case PQCAlgorithm::TEST_XOR:
        return PQCAlgorithmParams{"TEST-XOR", "none (test stand-in)", 32, 32, 64, 0};
    case PQCAlgorithm::UNKNOWN:
        return std::nullopt;
    }
    return std::nullopt;
}

} // namespace pqc
