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
- Keine Wallet-Integration, keine Skript-Interpreter-Aktivierung (Folge-Phasen);
  TEST_XOR parst niemals konsens-gültig (per Test bewiesen)
- Quantum-Sprache per §2/§38/§236 (SHA256d-Hinweis bleibt bestehen)
