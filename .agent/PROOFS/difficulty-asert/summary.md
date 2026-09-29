# PROOF — Phase 8 ASERT Difficulty (§39–§42, Erstdeliverable)

## Implementierung
- `src/consensus/params.h`: fUseASERT, nASERTHalfLife (172800), nASERTAnchorHeight (0),
  nASERTAnchorTime (= Genesis-Zeit), nASERTAnchorBits (= Genesis-nBits); Mainnet aktiviert
- `src/pow.cpp`: CalculateASERT (integer-exaktes aserti3-2d nach Norm-Pseudocode),
  GetNextWorkRequired-Branch (Tip-Evaluierung, pblock-unabhängig),
  PermittedDifficultyTransition ASERT-Ast (D-016)
- `contrib/quantbtc/ref/asert.py`: unabhängige Integer-Referenz + Float-Orakel
  (3000 Fälle < 2% Abweichung) + Vektorgenerator
- Testnets behalten Bitcoin-Retarget (differentielles Bett + weniger Churn)

## Evidenz (keine Mocks)
- 20/20 Differential-Vektoren C++ == Python (steady/fast/slow/crash/warp/
  clamps/negativ/huge/genesis-tip + 2 hard-anchor)
- C++: 867/868 PASS, 0 Failures, 26.556.192 Assertions (1 Debug-Warnung)
- Funktional: 11/11 PASS inkl. mining_basic (echte Nodes, Regtest-Pfad intakt)
- Fuzz-Targets pow/pow_transition/headerssync üben ASERT-Ast automatisch
  (MAIN-Params); Fuzz-Lauf selbst: NOT RUN (kein Clang-Setup lokal)

## Self-healing (ehrlich)
- F-ASERT-01: C++ int64-Overflow im Kubik-Polynom (stille falsche Targets!) —
  gefunden via Differential-Vektoren, behoben mit arith_uint256-Pfad
- F-ASERT-02: Test mit uint32-inkompatiblem Vektor (Jahr 2120) — Vektor falsch,
  nicht Code; ersetzt durch gültigen Max-Clamp-Vektor
- F-ASERT-03: Fallback-Anker (powLimit) ≠ Kompakt-Max numerisch — Testprämisse
  korrigiert (Boundedness statt Gleichheit)
- F-ASERT-04: pow_tests-2016-Vektoren vs. ASERT-Mainnet — NoAsert-Helper statt
  Vektor-Manipulation

## Bekannt / Nächste
- Echte Mainnet-ASERT-Validierung erst mit geminten Blöcken (Testnet-Phase);
  Live-Template-Check blockiert (keine Peers); Integer-Differential steht
- Nächste Phasen: PQC (§46–§55), Wallet, Rest; Clean-Checkout-Build ausstehend
