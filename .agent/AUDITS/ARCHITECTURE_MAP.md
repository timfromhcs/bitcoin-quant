# ARCHITECTURE_MAP — Baseline (2026-09-28)

- HEAD: `05bc2f53ce0cb239c17dbdd6b261bd2db7d2a940` (master == origin/master, Merge bitcoin/bitcoin#36024)
- Working tree: clean, einzige Ausnahme untracked `GEMINI.md` (Plan, 5564 Zeilen)
- Ergebnis: Fork ist code-identisch zu Bitcoin Core `32.99.0-dev`. Keine QuantBTC-/PQC-/NFT-/Storage-/Statepack-Implementierung vorhanden (nur `GEMINI.md` enthält diese Begriffe; `dilithium`/`sphincs`/`statepack` = 0 Treffer im Baum).

## Top-Level

| Pfad | Zweck |
|---|---|
| `src/` | C++-Core: Konsens, Validierung, P2P, Wallet, RPC, Qt, Krypto, bench, unit-tests |
| `test/` | Python-Funktionaltests (`functional/`, 285 Dateien), Lint, Fuzz-Runner |
| `contrib/` | Operator-/Dev-Tools (seeds, signet, guix, tracing, verify-binaries, zmq, devtools, linearize) |
| `doc/` | Build-Doku, JSON-RPC-Doku, design/, policy/, man/, release-notes/ |
| `share/` | Runtime-Assets (examples/bitcoin.conf, rpcauth, qt-Res, pixmaps, setup.nsi.in) |
| `depends/` | Legacy-Depends-System (parallel zu vcpkg/CMake) |
| `ci/` + `.github/` | CI-Skripte (33 Setup-Envs) + Workflow `ci.yml` |
| `cmake/` | CMake-Module (secp256k1, leveldb, minisketch, crc32c, Coverage) |

## src/-Unterstruktur (verifiziert vorhanden)

`bench/ common/ compat/ consensus/ crc32c/ crypto/ index/ init/ interfaces/ ipc/ kernel/ leveldb/ logging/ minisketch/ node/ policy/ primitives/ qt/ rpc/ script/ secp256k1/ support/ test/ univalue/ util/ wallet/ zmq/`

Abweichungen zum Plan-Schema (§18): kein `src/net/` (P2P flach: `net*.cpp/h`), kein `src/mining/` (Mining in `src/node/miner.*`, `mini_miner.*`, `block_template_manager.*` + `src/rpc/mining.*`), kein `src/signet/` (flach: `src/signet.*` + `src/kernel/signet.h`).

## Quant-Suche (Negativbefund, verifiziert)

- `quantbtc`: nur `GEMINI.md` (60), sonst 0
- `dilithium`/`sphincs`/`statepack`/`qsp`: 0 im Baum
- `pqc`/`nft`/`quant`-Treffer außerhalb GEMINI.md: geprüfte False-Positives (`quantity`, `benefit`, …)
