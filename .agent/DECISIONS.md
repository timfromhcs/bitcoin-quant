# DECISIONS.md — 2026-09-28

## D-001 — Token-Handhabung
- Problem: GitHub-PAT des Users darf nicht ins Repo/Backup
- Alternativen: (a) Remote-URL mit Token persistieren, (b) transienter `http.extraHeader` pro Push, (c) gh/credential-helper
- Gewählt: (b) — kein Secret in `.git/config`, `.agent/`, Backup oder Commits. Secret-Scan vor jedem Push (§131) bleibt Pflicht.
- Tests: vor Push `git diff --check`, Secret-Grep, Large-File-Scan

## D-002 — Build-Verzeichnis außerhalb des Repos
- Problem: Build-Artefakte dürfen das Repo nicht verschmutzen (§130)
- Gewählt: `E:/btc quant/build-probe/` (außerhalb), Logs zusätzlich in `.agent/BUILD_RESULTS/` + Backup `03-build/`

## D-003 — Difficulty-Kandidat: ASERT (DRAFT-Empfehlung, kein Freeze)
- Problem: Bitcoin-2016-Retarget ungeeignet für Low-Hashrate-Neustart (§41)
- Evidenz: `contrib/quantbtc/ref/difficulty_sim.py` + `.agent/RESEARCH/difficulty-evidence.md` (Closed-Loop, 5 Szenarien: ASERT erholt 10x-Crash auf Median 692 s vs. bitcoin 4127 s; LWMA warp-anfällig)
- Gewählt: ASERT mit 2-Tage-Halving als Konsens-Kandidat; Integer-Implementierung + Differentialtests + Adversarial-Tests vor Freeze Pflicht
- Tests: Simulations-Rerun deterministisch (LCG-seed)

## D-004 — Chain-Identity DRAFT-Werte (NICHT eingefroren)
- Problem: Unabhängige Netzidentität ohne Kollisionsrisiko (§25–§29)
- Gewählt: NUMS-Magics (SHA256-Label, siehe RESEARCH/network-identity-collisions.md), Ports 8444/8442-Familie, HRP `qb`, Base58 58/55 (Erstzeichen per Test zu beweisen), Datadir QuantBTC, chain_id `QBTC-1`
- Tests: `test_quantbtc_magic`, Cross-Network-Reject, Adress-Präfix-Tests vor Freeze

## D-005 — PQC-Kandidaten (NICHT eingefroren)
- Problem: PQ-Transaktionsautorisierung mit Standard-Rückhalt (§48)
- Evidenz: `.agent/RESEARCH/pqc-selection.md` (FIPS 204/205 final 2024-08-13; liboqs ML-DSA ≥0.12.0, SLH-DSA-Stand prüfen)
- Gewählt: Abstraktion zuerst (§46), ML-DSA-65 primär / SLH-DSA-SHA2-128s Backup, Freeze erst nach Build-/KAT-/Benchmark-Evidenz (§49)

## D-006 — Log-Encoding: UTF-8 Pflicht
- Problem: PowerShell-`>` schreibt UTF-16 → Git speichert Logs als binär (a8fb8a07b5 enthält 3 Binär-Logs; Inhalt intakt, aber unschön)
- Gewählt: künftig `Out-File -Encoding utf8` / Write-Tool; keine History-Umschreibung (Evidenz bleibt lesbar via `Get-Content`)

## D-007 — Funktionale Tests: Runner-Bypass mit ASCII-Tmpdir
- Problem: Runner-Emoji-Tmpdir + bin/Debug-Layout blockieren test_runner auf Windows (FAIL-001)
- Gewählt: Direktaufrufe mit `--configfile/--tmpdir` + Env-Binary-Overrides; kein Upstream-Patch nötig
