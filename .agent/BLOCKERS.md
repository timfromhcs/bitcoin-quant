# BLOCKERS.md — 2026-09-28 (Session 10: CI-Diagnose)

## B-003 — GitHub Actions: 0 Workflows registriert (BLOCKED-EXTERNAL)
- Symptom: `actions/workflows` → total 0; `actions/runs` → total 0; check-runs
  auf gepushten Commits → 0. Obwohl: `.github/workflows/ci.yml` existiert
  (Inhalt-API OK), push-Trigger `branches: ['**']`, Actions enabled=true,
  allowed=all, Repo public Fork.
- Diagnose (4 Endpunkte konsistent): GitHub hat KEINE Workflows registriert.
  Ursache serverseitig (Owner-Settings/Re-Verifizierung nötig), nicht im Code.
- Aktion: Repo-Owner prüft Settings → Actions (ggf. aus/an); nächster Push
  re-testet die Trigger-Hypothese automatisch. KEIN endloses Polling (§253).
- Kompensation: lokale Voll-Verifikation (Suite + Smoke + Clean-Build) bleibt
  die maßgebliche Gate-Evidenz bis CI läuft.


## B-001 — vcpkg/Boost-Provisionierung (GELÖST 2026-09-28, ENVIRONMENT/DEPENDENCY)
- Symptom: `cmake`-Configure brach ab (erst Boost missing, dann vcpkg-Baseline-Commit fehlend, dann Tool-Version zu alt, dann Qt-Scope zu groß)
- Root-Causes & Fixes: (1) `C:\vcpkg` existierte, PATH/VCPKG_ROOT fehlte → Toolchain-File explizit; (2) `git fetch` in C:\vcpkg für Baseline-Commit 9e593bb1; (3) Checkout 9e593bb1 + bootstrap → Tool 2026-07-27; (4) `VCPKG_MANIFEST_NO_DEFAULT_FEATURES=ON` + Features `wallet;zeromq` (Qt-Scope separat)
- Ergebnis: Configure PASS (142.9 s), `bitcoind` (Debug) gebaut + `--version` OK (v32.99.0). Logs: `.agent/BUILD_RESULTS/baseline-configure-attempt{2,3,5,6}.log`, `baseline-build-bitcoind.log`
- Rest: Voll-Build (Qt/Tests) + Baseline-Tests AUSSTEHEND (Qt-Installation läuft/h steht an)

## B-002 — Kein upstream-Remote (NOT VERIFIED ONLINE)
- Symptom: exakter Diff gegen bitcoin/bitcoin nicht per fetch belegbar
- Nächster Schritt: `git remote add upstream https://github.com/bitcoin/bitcoin.git` + fetch + `git merge-base`/Diff-Dokumentation (nur lesend)
- Blockiert: §20-Vergleich (als NOT VERIFIED ONLINE markiert, kein Release-Gate)

Update 2026-09-30: Re-Trigger nach Push 7293662e69 (60s gewartet) — weiterhin total_count=0. B-003 bleibt BLOCKED-external.
