# PROOF — Phase 6 Konsens-Isolation (§33–§35) + PoW-Suite (§36–§37)

## Entfernt/neutralisiert (CMainParams)
- script_flag_exceptions: leer (Bitcoin-Blöcke unerreichbar)
- BIP34Height=1 + BIP34Hash=null; BIP65/66/CSV/Segwit=1; MinBIP9WarningHeight=0
  (Testnet4-Konvention für frische Ketten; keine Bitcoin-Höhen mehr)
- Bereits phase-4/5: minWork/assumeValid/snapshots/TxStats leer, Genesis ersetzt
- BEHALTEN (dokumentiert): Headersync-Tuning (Null crasht per Assert),
  Subsidy-Schedule (Geldpolitik-Phase), powLimit/Spacing (Difficulty-Phase)

## PoW-Suite (§37)
- quantbtc_tests::pow_negative: Compact-Dekodierung 0x1d00ffff == Difficulty-1-
  Target, mutierte Nonce FAIL, 256x-härteres Target FAIL, Overflow/Negativ FAIL
- Python-Referenz deckt dieselben Fälle (Differential-Anker §114)

## Tests
- C++: 865/866 PASS, 0 Failures, 27.029.716 Assertions (1 Debug-Warnung)
- Funktional: 10/10 PASS (getchainparams-Mainnet-Genesis mitgezogen)
- miner_tests-Heilung: Regtest + CSV-Deferral + Grinding + Witness-Drop +
  wandnahe Zeiten (Details: JOURNAL Session 4)

## Bekannt
- Voll-Suite (285) NOT RUN; GUI-Runtime NOT RUN; Difficulty/PQC ausstehend
