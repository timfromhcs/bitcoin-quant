# PROOF — Phase 4 Chain Identity (2026-09-28, Branch feat/chain-identity)

## Claim
Der Mainnet-Knoten trägt keine aktive Bitcoin-Netzidentität mehr; alle Netze
nutzen QuantBTC-Magics/Ports/Präfixe; keine Bitcoin-Seeds/Snapshots/Trust-Anker.

## Änderungen (Auswahl)
- `src/kernel/chainparams.{h,cpp}`: `m_chain_id="QBTC-1"` + Accessor; Main-Magic
  `3c596652`, Port 8444; Testnet3 `c89d0720`/28444; Testnet4 `b1ad6e5f`/29444;
  Regtest `c61e7b95`/28445; Signet-Port 38445; Seeds/FixedSeeds leer (außer
  regtest-dummy); minWork/assumeValid/snapshots/TxStats leer (außer regtest-
  Test-Fixtures, dokumentiert); Adresspräfixe Main 58/120/`qb`/`qsp`,
  Test 112/196/`tqb`/`tqsp`, Regtest 112/196/`qbrt`/`qsprt`; EXT-Keys per D-008
  beibehalten; Genesis-Mechanik temporär bis Genesis-Phase
- `src/chainparamsbase.cpp`: RPC-Ports 8442/28442/29442/38442/28443
- `src/common/args.cpp`: Datadir QuantBTC/`~/.quantbtc/`
- `src/key_io.cpp`: Bech32-Probe + Base58-Fallback (D-009)
- `src/bip324.cpp`: Salt `quantbtc_v2_shared_secret` + re-verankerte Vektoren
- Tests: neu `src/test/quantbtc_tests.cpp` (6 Cases); bip32/bip352/descriptor/
  key_io/key/net_peer/rpc/script_standard/util/chainstatemanager-Vektoren
  payload-erhaltend re-verankert; Framework-Magics/Adress-Helpers/Daten-JSONs
  synchronisiert

## Tests (alle mit Evidenz, keine Mocks)
- C++-Suite: 863/864 PASS, 1 Warnung (Debug-Leak-Artefakt), 27.066.361/27.066.361
  Assertions, 0 Failures
- Funktional (echte Nodes): example_test, interface_rpc, rpc_blockchain,
  wallet_basic, feature_assumevalid, p2p_handshake, feature_config_args,
  wallet_disable, rpc_generate, tool_utils — 10/10 PASS
- Python-Referenz: 11/11 PASS (unverändert)

## Bekannte Limitierungen
- Genesis-Blöcke noch Bitcoin-Mechanik (Phase 5)
- MESSAGE_MAGIC + Signet-Challenge + Testnet-P2SH-196 geteilt (D-010)
- GUI-Runtime-QA headless NOT RUN; Voll-Funktional-Suite (285) NOT RUN

## Rollback
Branch feat/chain-identity; Baseline-Bundle + Commits auf
origin/feat/audit-baseline (a8fb8a07b5 + f23a57384a).
