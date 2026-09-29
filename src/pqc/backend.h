// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QUANTBTC_PQC_BACKEND_H
#define QUANTBTC_PQC_BACKEND_H

#include <pqc/algorithm.h>
#include <pqc/error.h>
#include <pqc/key.h>

#include <span>
#include <util/expected.h>

#include <memory>
#include <string>
#include <vector>

namespace pqc {

/** Cryptographic backend interface (§47). Implementations must never mix
 * algorithms: every call carries its algorithm tag and sizes are enforced
 * against GetAlgorithmParams. */
class PQCBackend {
public:
    virtual ~PQCBackend() = default;
    virtual const char* Name() const = 0;
    /** True for test stand-ins: consensus validation must reject their output. */
    virtual bool IsTestOnly() const = 0;
    virtual bool Supports(PQCAlgorithm algo) const = 0;
    virtual util::Expected<PQCKeyPair, PQCError> Generate(PQCAlgorithm algo) = 0;
    virtual util::Expected<PQCSignature, PQCError> Sign(const PQCKeyPair& key, std::span<const unsigned char> msg) = 0;
    virtual PQCError Verify(std::span<const unsigned char> pubkey, std::span<const unsigned char> msg, const PQCSignature& sig) = 0;
};

/** Global backend registry. Register once at startup/tests; lookup by name
 * ("test", "liboqs"). Not consensus-critical state (backends are code). */
void RegisterPQCBackend(std::shared_ptr<PQCBackend> backend);
std::shared_ptr<PQCBackend> GetPQCBackend(const std::string& name);
std::vector<std::string> ListPQCBackends();

/** Register the test backend always, plus the liboqs backend when compiled
 * with HAVE_LIBOQS. Called by unit tests and node init (phase 10+). */
void RegisterDefaultPQCBackends();

} // namespace pqc

#endif // QUANTBTC_PQC_BACKEND_H
