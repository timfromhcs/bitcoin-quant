// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pqc/backend_oqs.h>

#ifdef HAVE_LIBOQS
#include <oqs/oqs.h>
#endif

namespace pqc {

const char* OqsBackend::OqsAlgName(PQCAlgorithm algo)
{
    if (algo == PQCAlgorithm::ML_DSA_65) return "ML-DSA-65";
    return nullptr; // SLH-DSA-*: reserved until the integration is verified
}

const char* OqsBackend::OqsVersion()
{
#ifdef HAVE_LIBOQS
    return OQS_VERSION_TEXT;
#else
    return "not compiled";
#endif
}

bool OqsBackend::Supports(PQCAlgorithm algo) const
{
#ifdef HAVE_LIBOQS
    if (OqsAlgName(algo) == nullptr) return false;
    OQS_SIG* sig{OQS_SIG_new(OqsAlgName(algo))};
    if (sig == nullptr) {
        // Older liboqs without the final FIPS 204 names: try legacy alias.
        if (algo == PQCAlgorithm::ML_DSA_65) sig = OQS_SIG_new("Dilithium3");
        if (sig == nullptr) return false;
    }
    OQS_SIG_free(sig);
    return true;
#else
    return false;
#endif
}

util::Expected<PQCKeyPair, PQCError> OqsBackend::Generate(PQCAlgorithm algo)
{
#ifdef HAVE_LIBOQS
    const char* name{OqsAlgName(algo)};
    if (name == nullptr) return util::Unexpected(PQCError::UNSUPPORTED_ALGORITHM);
    const auto params{GetAlgorithmParams(algo)};
    if (!params) return util::Unexpected(PQCError::UNKNOWN_ALGORITHM);
    OQS_SIG* sig{OQS_SIG_new(name)};
    if (sig == nullptr && algo == PQCAlgorithm::ML_DSA_65) sig = OQS_SIG_new("Dilithium3");
    if (sig == nullptr) return util::Unexpected(PQCError::BACKEND_MISSING);
    // Cross-check library sizes against the normative parameters (FIPS 204).
    if (sig->length_public_key != params->pubkey_size ||
        sig->length_secret_key != params->privkey_size ||
        sig->length_signature != params->sig_size) {
        OQS_SIG_free(sig);
        return util::Unexpected(PQCError::INTERNAL);
    }
    PQCKeyPair kp;
    kp.algo = algo;
    kp.pubkey.resize(params->pubkey_size);
    kp.privkey.resize(params->privkey_size);
    if (OQS_SIG_keypair(sig, kp.pubkey.data(), kp.privkey.data()) != OQS_SUCCESS) {
        OQS_SIG_free(sig);
        return util::Unexpected(PQCError::INTERNAL);
    }
    OQS_SIG_free(sig);
    return kp;
#else
    return util::Unexpected(PQCError::BACKEND_MISSING);
#endif
}

util::Expected<PQCSignature, PQCError> OqsBackend::Sign(const PQCKeyPair& key, std::span<const unsigned char> msg)
{
#ifdef HAVE_LIBOQS
    const char* name{OqsAlgName(key.algo)};
    if (name == nullptr || !key.IsValid()) return util::Unexpected(PQCError::WRONG_KEY);
    const auto params{GetAlgorithmParams(key.algo)};
    if (!params || key.pubkey.size() != params->pubkey_size || key.privkey.size() != params->privkey_size) {
        return util::Unexpected(PQCError::BAD_SIZE);
    }
    OQS_SIG* sig{OQS_SIG_new(name)};
    if (sig == nullptr && key.algo == PQCAlgorithm::ML_DSA_65) sig = OQS_SIG_new("Dilithium3");
    if (sig == nullptr) return util::Unexpected(PQCError::BACKEND_MISSING);
    PQCSignature out;
    out.algo = key.algo;
    out.bytes.resize(params->sig_size);
    size_t sig_len{0};
    // Empty context string = FIPS 204 default.
    if (OQS_SIG_sign(sig, out.bytes.data(), &sig_len, msg.data(), msg.size(), key.privkey.data()) != OQS_SUCCESS) {
        OQS_SIG_free(sig);
        return util::Unexpected(PQCError::INTERNAL);
    }
    out.bytes.resize(sig_len);
    OQS_SIG_free(sig);
    return out;
#else
    return util::Unexpected(PQCError::BACKEND_MISSING);
#endif
}

PQCError OqsBackend::Verify(std::span<const unsigned char> pubkey, std::span<const unsigned char> msg, const PQCSignature& sig)
{
#ifdef HAVE_LIBOQS
    const char* name{OqsAlgName(sig.algo)};
    if (name == nullptr) return PQCError::UNSUPPORTED_ALGORITHM;
    const auto params{GetAlgorithmParams(sig.algo)};
    if (!params) return PQCError::UNKNOWN_ALGORITHM;
    if (pubkey.size() != params->pubkey_size) return PQCError::BAD_SIZE;
    if (sig.bytes.size() != params->sig_size) return PQCError::BAD_SIZE;
    OQS_SIG* oqs{OQS_SIG_new(name)};
    if (oqs == nullptr && sig.algo == PQCAlgorithm::ML_DSA_65) oqs = OQS_SIG_new("Dilithium3");
    if (oqs == nullptr) return PQCError::BACKEND_MISSING;
    // Empty context string = FIPS 204 default (matches Sign above).
    const OQS_STATUS rc{OQS_SIG_verify(oqs, msg.data(), msg.size(), sig.bytes.data(), sig.bytes.size(), pubkey.data())};
    OQS_SIG_free(oqs);
    return rc == OQS_SUCCESS ? PQCError::OK : PQCError::VERIFY_FAILED;
#else
    return PQCError::BACKEND_MISSING;
#endif
}

} // namespace pqc
