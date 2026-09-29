// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QUANTBTC_PQC_BACKEND_TEST_H
#define QUANTBTC_PQC_BACKEND_TEST_H

#include <pqc/backend.h>

namespace pqc {

/** Deterministic test stand-in (§47). NOT cryptography: signatures are
 * SHA256 domain constructions anyone can recompute, offering zero
 * unforgeability. Purpose: exercise envelopes, registry, wallet plumbing and
 * negative paths without a crypto dependency. Serves TEST_XOR only;
 * consensus validation must reject TEST_XOR (enforced by encoding layer). */
class TestBackend : public PQCBackend {
public:
    const char* Name() const override { return "test"; }
    bool IsTestOnly() const override { return true; }
    bool Supports(PQCAlgorithm algo) const override { return algo == PQCAlgorithm::TEST_XOR; }
    util::Expected<PQCKeyPair, PQCError> Generate(PQCAlgorithm algo) override;
    util::Expected<PQCSignature, PQCError> Sign(const PQCKeyPair& key, std::span<const unsigned char> msg) override;
    PQCError Verify(std::span<const unsigned char> pubkey, std::span<const unsigned char> msg, const PQCSignature& sig) override;
};

} // namespace pqc

#endif // QUANTBTC_PQC_BACKEND_TEST_H
