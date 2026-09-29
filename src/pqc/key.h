// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QUANTBTC_PQC_KEY_H
#define QUANTBTC_PQC_KEY_H

#include <pqc/algorithm.h>
#include <support/allocators/secure.h>
#include <support/cleanse.h>

#include <vector>

namespace pqc {

/** Raw post-quantum key pair. Private material uses a zeroizing allocator;
 * lifetimes must still be kept explicit (lock wallet when idle, never log).
 * Platform limitations (mlock, swap) are documented, not assumed (§54). */
struct PQCKeyPair {
    PQCAlgorithm algo{PQCAlgorithm::UNKNOWN};
    std::vector<unsigned char> pubkey; //!< raw public key bytes
    std::vector<unsigned char, secure_allocator<unsigned char>> privkey; //!< raw secret bytes

    bool IsValid() const { return algo != PQCAlgorithm::UNKNOWN && !pubkey.empty() && !privkey.empty(); }
    void Clear()
    {
        algo = PQCAlgorithm::UNKNOWN;
        pubkey.clear();
        privkey.clear();
    }
};

/** A post-quantum signature: algorithm tag + raw signature bytes. */
struct PQCSignature {
    PQCAlgorithm algo{PQCAlgorithm::UNKNOWN};
    std::vector<unsigned char> bytes; //!< raw signature bytes

    bool IsValid() const { return algo != PQCAlgorithm::UNKNOWN && !bytes.empty(); }
};

} // namespace pqc

#endif // QUANTBTC_PQC_KEY_H
