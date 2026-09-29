// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pqc/error.h>

namespace pqc {

const char* PQCErrorString(PQCError err)
{
    switch (err) {
    case PQCError::OK: return "ok";
    case PQCError::UNKNOWN_ALGORITHM: return "unknown PQC algorithm";
    case PQCError::UNSUPPORTED_ALGORITHM: return "PQC algorithm recognized but no backend available";
    case PQCError::BAD_VERSION: return "unsupported PQC envelope version";
    case PQCError::BAD_SIZE: return "PQC key/signature/blob size violates algorithm parameters";
    case PQCError::BAD_ENCODING: return "malformed PQC envelope or script structure";
    case PQCError::WRONG_KEY: return "public key does not match commitment or algorithm";
    case PQCError::VERIFY_FAILED: return "post-quantum signature verification failed";
    case PQCError::TEST_ONLY: return "test-only PQC material where consensus material is required";
    case PQCError::BACKEND_MISSING: return "requested PQC backend not compiled or registered";
    case PQCError::INTERNAL: return "PQC backend invariant violation";
    }
    return "unrecognized PQC error";
}

} // namespace pqc
