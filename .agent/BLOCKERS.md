# BLOCKERS.md — 2026-09-28

## B-001 — vcpkg/Boost-Provisionierung (BUILD-BLOCKER, ENVIRONMENT/DEPENDENCY BUG)
- Symptom: `cmake`-Configure bricht ab (`AddBoostIfNeeded.cmake:32`, Boost 1.74.0 nicht gefunden). Log: `.agent/BUILD_RESULTS/baseline-configure-probe.log`
- Ursache: `VCPKG_ROOT` leer, keine provisionierten Deps
- Nächster Schritt: vcpkg bootstrap + `vcpkg install` gemäß `vcpkg.json` (Baseline 9e593bb1, 2026-07-29) ODER depends-Build; danach Configure/Build/Test erneut
- Freigabe-Kriterium: `cmake`-Configure vollständig ohne Fehler

## B-002 — Kein upstream-Remote (NOT VERIFIED ONLINE)
- Symptom: exakter Diff gegen bitcoin/bitcoin nicht per fetch belegbar
- Nächster Schritt: `git remote add upstream https://github.com/bitcoin/bitcoin.git` + fetch + `git merge-base`/Diff-Dokumentation (nur lesend)
- Blockiert: §20-Vergleich (als NOT VERIFIED ONLINE markiert, kein Release-Gate)
