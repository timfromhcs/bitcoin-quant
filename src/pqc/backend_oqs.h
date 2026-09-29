// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QUANTBTC_PQC_BACKEND_OQS_H
#define QUANTBTC_PQC_BACKEND_OQS_H

#include <pqc/backend.h>

namespace pqc {

/** Production backend over liboqs (§48): ML-DSA-65 (FIPS 204). System RNG via
 * liboqs; empty FIPS context (the standard default). Sizes cross-checked
 * against GetAlgorithmParams on first use. */
class OqsBackend : public PQCBackend {
public:
    const char* Name() const override { return "liboqs"; }
    bool IsTestOnly() const override { return false; }
    bool Supports(PQCAlgorithm algo) const override;
    util::Expected<PQCKeyPair, PQCError> Generate(PQCAlgorithm algo) override;
    util::Expected<PQCSignature, PQCError> Sign(const PQCKeyPair& key, std::span<const unsigned char> msg) override;
    PQCError Verify(std::span<const unsigned char> pubkey, std::span<const unsigned char> msg, const PQCSignature& sig) override;
    /** Canonical liboqs algorithm string for an id, or nullptr if none. */
    static const char* OqsAlgName(PQCAlgorithm algo);
    /** liboqs build version this backend was compiled against. */
    static const char* OqsVersion();
};

} // namespace pqc

#endif // QUANTBTC_PQC_BACKEND_OQS_H
