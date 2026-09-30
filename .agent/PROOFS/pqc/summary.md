# PROOF — Phase 9 PQC Abstraktion + Backends (§46–§52, Erstdeliverable)

## Geliefert
- `src/pqc/`: algorithm (IDs/Parameter), error (Modell §93), key (zeroizing),
  encoding (versionierte Envelopes + P2PQ build/parse/commitment),
  backend (Interface + Registry), backend_test (deterministisch, TEST_XOR),
  backend_oqs (liboqs ML-DSA-65, FIPS-204-Default, Größen-Kreuzcheck)
- vcpkg-`pqc`-Feature (liboqs 0.15.0); CMake: `pqc`-Lib + HAVE_LIBOQS-Public
- `src/bench/pqc.cpp`: Keygen/Sign/Verify-Benchmarks (permanent, §49/§160)
- Tests `src/test/pqc_tests.cpp` (7 Cases) + `src/test/data/mldsa65_kat.json`
  (NIST ACVP, Provenienz dokumentiert)

## Evidenz (keine Mocks für Real-Claims)
- NIST ACVP ML-DSA-sigVer-Vektor verifiziert OK + Bitflip-Negative FAIL
  (externe Konformität, unabhängige Quelle)
- Round-Trips + volle Negativ-Batterie (msg/sig/key/algo/trunkiert/übergroß/malformiert)
- Größen: pk 1952 / sk 4032 / sig 3309 (FIPS 204, extern bestätigt)
- Benchmarks (Debug, Win11/AMD64, MSVC, Commit s. unten): Keygen 573µs,
  Sign ~1,6ms, Verify 599µs; Release-Werte ausstehend (§161-Hinweis)
- C++: 874/875 PASS (0 Failures, 27.096.742 Assertions); funktional 5/5 Stichprobe
- liboqs-Version: im Test-Log protokolliert (OqsVersion), MIT-Lizenz verifiziert

## Ausdrücklich NICHT behauptet
- Keine FIPS-Konformität aus Tests allein (OQS-Audit-Warnung gilt)
- Kein SLH-DSA-Backend (reserviert, Format-Parameter bekannt)
- Keine Wallet-Integration (Keystore/RPC/GUI = Folge-Phase); Mempool-Policy
  für P2PQ steht aus (Konsens-Pfad bewiesen, Relay-Policy folgt)
- Quantum-Sprache per §2/§38/§236 (SHA256d-Hinweis bleibt bestehen)

## Nachtrag Wallet-Layer (Phase 10b)
- `p2pq(<hex>)`-Deskriptor (parse/expand/round-trip/Negative, C++-getestet)
- Funktional `wallet_pqc.py`: echter importdescriptors + Funding + Tracking mit
  exakten Konsens-Bytes; `solvable=false` bewiesen (kein Custody-Anschein)
- C++: 876/877 PASS (0 Failures, 26.768.562 Assertions)
- Offen: PQC-Keystore, SignStep-Hook, RPC-Send, Policy, Mempool-Tests

## Nachtrag Skript-Layer (Phase 10a)
- `CheckPQCSignature`-Virtual + `VerifyWitnessProgram`-Ast (v1/35B):
  Witness [sig_blob, pubkey_blob], BIP143-SIGHASH_ALL-Bindung, fail-closed
- E2E `p2pq_spend_e2e`: echter VerifyScript-Spend (liboqs über echter Sighash)
  + Negative (mutiert/falscher Amount/leer/kurz)
- C++: 875/876 PASS (0 Failures, 26.663.939 Assertions); funktional 13/13
  inkl. segwit/taproot (Witness-Pfade intakt)
