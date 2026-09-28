# Functional Smoke — Baseline (2026-09-28, real nodes, PASS 4/4)

Methode: Tests direkt aufgerufen (Bypass test_runner wegen Emoji-Tmpdir, s. FAILURES),
`--configfile=test/config.ini --tmpdir=C:\ft\smokeN`, Binaries via BITCOIND/-CLI/-TX/-WALLET/-UTIL/-BIN
auf `build-full/bin/Debug/`, `PYTHONUTF8=1`.

| Test | Ergebnis | Bemerkung |
|---|---|---|
| example_test | PASS | echte Nodes, Block-Propagation verifiziert |
| feature_config_args | PASS | |
| interface_rpc | PASS | |
| rpc_blockchain | PASS | |
| wallet_basic | PASS | inkl. Wallet-Signaturpfad |

Harness-Healing (3 Root-Causes, alle ENVIRONMENT, alle repariert/umgangen):
1. test_runner erzielte `test_runner_₿_🏃_`-Tmpdir → Mojibake-Pfad → bitcoind-Start FAILT.
   Fix: Tests direkt mit ASCII-`--tmpdir` aufrufen.
2. Framework-Default `BUILDDIR/bin/*.exe` vs. CMake-Multi-Config `bin/Debug/*.exe` → FileNotFound.
   Fix: dokumentierte Env-Overrides (BITCOIND, BITCOINCLI, BITCOINTX, BITCOINWALLET, BITCOINUTIL, BITCOIN_BIN).
3. Ergebnis-Printer crasht unter cp1252 (✖-Zeichen). Fix: `PYTHONUTF8=1`.
4. Ein transienter Popen-Fehlstart (einmalig, danach 3/3 stabil) → RACE/FLAKY-Umgebung, kein Code-Bug.

Vollsuite (285 Tests): NOT RUN (Stunden-Budget, Folgelauf).
