// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QUANTBTC_PQC_ERROR_H
#define QUANTBTC_PQC_ERROR_H

namespace pqc {

/** Machine-readable PQC error model (§93). Every user-visible failure maps
 * these to technical message + user message + suggested action at the RPC/GUI
 * layer (phase 10+); the core only produces codes. */
enum class PQCError {
    OK = 0,
    UNKNOWN_ALGORITHM,   //!< algorithm id not registered/recognized
    UNSUPPORTED_ALGORITHM, //!< recognized but no backend available
    BAD_VERSION,         //!< envelope version mismatch (witness/serialization)
    BAD_SIZE,            //!< key/signature/blob size violates algorithm params
    BAD_ENCODING,        //!< malformed envelope or script structure
    WRONG_KEY,           //!< pubkey does not match commitment / wrong algorithm key
    VERIFY_FAILED,       //!< cryptographic verification failed
    TEST_ONLY,           //!< test-only material presented where consensus material required
    BACKEND_MISSING,     //!< requested backend not compiled/registered
    INTERNAL,            //!< backend invariant violation (bug)
};

const char* PQCErrorString(PQCError err);

} // namespace pqc

#endif // QUANTBTC_PQC_ERROR_H
