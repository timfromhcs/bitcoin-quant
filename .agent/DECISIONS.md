# DECISIONS.md — 2026-09-28

## D-001 — Token-Handhabung
- Problem: GitHub-PAT des Users darf nicht ins Repo/Backup
- Alternativen: (a) Remote-URL mit Token persistieren, (b) transienter `http.extraHeader` pro Push, (c) gh/credential-helper
- Gewählt: (b) — kein Secret in `.git/config`, `.agent/`, Backup oder Commits. Secret-Scan vor jedem Push (§131) bleibt Pflicht.
- Tests: vor Push `git diff --check`, Secret-Grep, Large-File-Scan

## D-002 — Build-Verzeichnis außerhalb des Repos
- Problem: Build-Artefakte dürfen das Repo nicht verschmutzen (§130)
- Gewählt: `E:/btc quant/build-probe/` (außerhalb), Logs zusätzlich in `.agent/BUILD_RESULTS/` + Backup `03-build/`
