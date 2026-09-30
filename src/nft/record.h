// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_NFT_RECORD_H
#define BITCOIN_NFT_RECORD_H

#include <uint256.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace nft {
//! NFT mint-record core (QuantBTC protocol v1, spec §11).
//! Pure record codec + deterministic id. No registry, no transfers, no
//! consensus, no I/O — same DAG rule as storage (links bitcoin_crypto only).
//! NOTE: enum values are CamelCase on purpose (OVERFLOW taught us that
//! SCREAMING_CASE collides with platform macros).
static constexpr unsigned char MINT_RECORD_VERSION = 1;
static constexpr size_t MAX_OWNER_SCRIPT_SIZE = 10000;

struct MintRecord {
    uint256 collection_id;
    uint64_t serial{0};
    uint256 content_root;
    uint256 metadata_hash;
    std::vector<unsigned char> owner_script;
    uint64_t minted_epoch{0};
};

enum class MintError {
    Ok,
    TooShort,
    BadVersion,
    BadScriptSize,
};

//! Deterministic asset id: SHA256d(collection_id || BE64(serial)).
//! Uniqueness is a registry property; the formula only binds the fields.
uint256 NftId(const uint256& collection_id, uint64_t serial);

//! Strict decode: exact length match, version gate, structural validation.
MintError DecodeMint(const unsigned char* data, size_t len, MintRecord& out);

//! Encode (precondition: structurally valid record; decode validates).
std::vector<unsigned char> EncodeMint(const MintRecord& rec);
} // namespace nft

#endif // BITCOIN_NFT_RECORD_H
