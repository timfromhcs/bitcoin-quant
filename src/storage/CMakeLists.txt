# Copyright (c) 2026-present The QuantBTC developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

# Storage proof-of-retrievability core (QuantBTC protocol v1).
# Pure functions only: links bitcoin_crypto (SHA256), never consensus
# (the future consensus hook will call inward: consensus -> storage).
add_library(storage STATIC EXCLUDE_FROM_ALL
  proof.cpp
)
target_link_libraries(storage
  PRIVATE
    bitcoin_crypto
)
