# Copyright (c) 2026-present The QuantBTC developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

# Post-quantum cryptography abstraction (QuantBTC protocol v1).
# Backends: deterministic test stand-in (always) + liboqs production
# backend (only when find_package(liboqs) above succeeded).
set(PQC_SOURCES
  algorithm.cpp
  backend.cpp
  backend_test.cpp
  encoding.cpp
  error.cpp
)
if(TARGET OQS::oqs)
  list(APPEND PQC_SOURCES backend_oqs.cpp)
endif()
add_library(pqc STATIC EXCLUDE_FROM_ALL ${PQC_SOURCES})
target_link_libraries(pqc
  PRIVATE
    core_interface
    bitcoin_consensus
    bitcoin_util
)
if(TARGET OQS::oqs)
  target_link_libraries(pqc PRIVATE OQS::oqs)
  # PUBLIC so dependents (tests, node) see the same backend selection.
  target_compile_definitions(pqc PUBLIC HAVE_LIBOQS)
endif()
