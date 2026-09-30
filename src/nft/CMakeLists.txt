# Copyright (c) 2026-present The QuantBTC developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

# NFT mint-record core (QuantBTC protocol v1).
# Pure record codec: links bitcoin_crypto (SHA256) only, never consensus.
add_library(nft STATIC EXCLUDE_FROM_ALL
  record.cpp
)
target_link_libraries(nft
  PRIVATE
    bitcoin_crypto
)
