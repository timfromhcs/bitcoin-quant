# TEST_MAP — Baseline (2026-09-28)

| Suite | Lage | Umfang | Wichtigste Ziele |
|---|---|---|---|
| Unit (Boost) | `src/test/*.cpp` | 136 Dateien | `validation_*`, `coins_tests`, `pow_tests`, `merkle_tests`, `script_tests`, `miniscript_tests`, `crypto_tests`, `key_tests`, `mempool_tests`, `net_tests`, `miner_tests`, `psbt_tests`, `descriptor_tests`, `bip32/324/352_tests`, `addrman/banman/peerman_tests` |
| Funktional (Python) | `test/functional/*.py` | 285 Dateien | Runner `test_runner.py`, `feature_block.py`, `feature_assumeutxo/assumevalid.py`, ~50 `wallet_*.py`, `rpc_*.py`, `p2p_*.py`, `mempool_*.py`, `tool_*.py` |
| Fuzz | `src/test/fuzz/*.cpp` | 142 Dateien | `fuzz.*`, `FuzzedDataProvider.h`, `kitchen_sink.cpp`, block/connect_block/deserialize/eval_script/key/miniscript/net/p2p_handshake/crypto_*/txdownloadman; `test/fuzz/` nur noch `test_runner.py` |
| Bench | `src/bench/*.cpp` | 60 Dateien | `bench_bitcoin.cpp`, `connectblock.cpp`, `checkblock*`, `coin_selection`, `crypto_hash`, `verify_script`, `mempool_*`, `wallet_*` |
| Lint | `test/lint/*` | 17 Einträge | `lint-files.py`, `lint-includes.py`, `lint-include-guards.py`, `lint-circular-dependencies.py`, `lint-python.py`, `lint-shell{,-locale}.py`, `lint-tests.py`, `check-doc.py`, `lint-qt-translation.py`, `lint-locale-dependence.py`, Commit-/Subtree-Checks |

Status: alle Suiten = NOT RUN (kein Build vorhanden).
