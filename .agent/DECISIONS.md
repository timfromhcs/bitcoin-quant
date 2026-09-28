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

## D-008 — Extended-Key-Versionen bleiben BIP32-kompatibel (xpub/tpub)
- Problem: Eigene EXT-Präfixe brachen bip32_tests (BIP32-Standardvektoren nutzen xpub/xprv)
- Alternativen: (a) eigene Präfixe + Standardvektoren ändern (verboten, §16), (b) xpub/tpub behalten
- Gewählt: (b) — EXT-Versionen sind Serialisierungs-Labels (BIP32-Interop mit HW-Wallets/Tools), Isolation gilt für Adressen/Magics/Ports/Seeds. Restrisiko (Watch-only-Verwechslung) dokumentiert, GUI-Warnung folgt in Wallet-Phase
- Tests: bip32_tests unverändert grün

## D-009 — DecodeDestination mit Bech32-Probe + Base58-Fallback
- Problem: Echte Kollision — neue `Q…`-P2PKH-Adressen beginnen case-insensitiv mit HRP `qb` und wurden als Bech32 fehlklassifiziert (CONSENSUS-nah: Adress-Dekodierung)
- Gewählt: HRP-Match löst Bech32-Probe aus; nur bei gültigem Bech32-Decode gilt Bech32-Pfad, sonst Base58-Fallback. Alle bisherigen Fehlerpfade für echte Bech32-Inputs unverändert
- Tests: key_io_valid/invalid, descriptor, bip352, quantbtc grün; voll-Suite 863/864

## D-010 — Zurückgestellt (dokumentiert, nicht vergessen)
- (a) MESSAGE_MAGIC bleibt vorerst `"Bitcoin Signed Message:\n"` — Cross-Chain-Replay-Risiko für signmessage als KNOWN LIMITATION; Migration mit neuen Vektoren in Wallet-Phase (Privkeys der Vektoren unbekannt, daher kein spontaner Wechsel)
- (b) Signet-Default-Challenge bleibt Bitcoin's (Magic dynamisch); eigene Signet-Challenge in P2P/Testnet-Phase
- (c) Testnet-P2SH-Präfix 196 geteilt (führendes `2` wie Bitcoin-Testnet); Mainnet isoliert (`q`)
- (d) Regtest-AssumeUTXO-Fixtures (Höhen 110/200/299) bleiben (Test-Fixtures, kein Trust-Anker)

## D-011 — Keine Datei-Edits via Shell-Redirects
- Problem: PowerShell-`io.open(f,'w')`-Einzeiler hat 3 Framework-Dateien geleert (Auswertungsreihenfolge trunkiert vor Read) — FAIL-002, per Git wiederhergestellt, kein Datenverlust
- Gewählt: Datei-Edits AUSSCHLIESSLICH via Edit/Write-Tools; Shell nur für read-only + Builds + git

## D-012 — Genesis: NUMS-Key + Timestamp-Roll statt schwächerer Difficulty
- Problem: Nonce-Space bei Difficulty 1 war leer (P≈37%); Key-Zeremonie unerwünscht
- Gewählt: (a) NUMS-Pubkey (SHA256-Grinding, Counter 0, komprimiert, für jeden verifizierbar); (b) begrenzter Timestamp-Roll (MAX_ROLL 3600 s) statt nBits-Absenkung (powLimit bleibt Difficulty 1); (c) Chunk-Ledger-Parallel-Mining (16 Worker, global-minimal, schedulings-unabhängig, resumable)
- Gefunden via C++-Assert: Generator nutzte stale 0x41-Push für 33B-Key (Konsens: 0x21) — Serialisierungs-Konstanten immer gegen C++ kreuzprüfen
- Tests: Doppel-Generierung identisch; C++-Assert fail-closed; bitcoind-Mainnet verifiziert
