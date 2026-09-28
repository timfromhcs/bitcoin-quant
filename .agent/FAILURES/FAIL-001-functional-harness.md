# FAIL-001 — Funktionaler Harness startet keine Nodes (GELÖST, ENVIRONMENT)

## Symptome
1. `test_runner.py` → `create_cache.py` CalledProcessError (Exit 1).
2. Direkter `example_test.py` → `FileNotFoundError` in `test_node.py:317 Popen`.
3. `print_results` → `UnicodeEncodeError` (cp1252, ✖).

## Klassifikation
- (1): ENVIRONMENT BUG — Runner-Tmpdir `test_runner_₿_🏃_<ts>` wird auf diesem System zu Mojibake (`���`), bitcoind startet unter dem Pfad nicht.
- (2): ENVIRONMENT BUG — Framework-Defaultpfad `BUILDDIR/bin/*.exe` existiert bei CMake-Multi-Config nicht (real: `bin/Debug/*.exe`).
- (3): ENVIRONMENT BUG — Konsolen-Encoding.
- Ein einzelner transienter Popen-Fehlstart vor der Fix-Serie: RACE/FLAKY (danach 3/3 stabil).

## Reparatur
- Tests direkt mit ASCII-`--tmpdir` + `--configfile` aufrufen (Runner-Bypass dokumentiert).
- Binaries per Env-Override setzen (herstellerdokumentiert in `util.py:get_binary_paths`).
- `PYTHONUTF8=1` setzen.
- Regression: Smoke-Subset 4/4 + example_test PASS (TEST_RESULTS/functional-smoke.md).

## Offen
- Runner-Emoji-Tmpdir upstream melden? Upstream-CI (Linux) unbetroffen; Windows-lokal dokumentiert. Keine Code-Änderung nötig.
