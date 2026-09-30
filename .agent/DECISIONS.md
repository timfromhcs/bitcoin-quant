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

## D-013 — Konsens-Isolation: Fresh-Chain-Konvention + Headersync behalten
- Problem: Bitcoin-Aktivierungshistorie (BIP34/65/66/CSV/Segwit-Höhen, Script-Exceptions, alter BIP34Hash) auf neuer Kette (§33)
- Gewählt: Alles ab Höhe 1 aktiv + BIP34Hash null + Exceptions leer (exakte Testnet4-Konvention für frische Ketten); Headersync-Tuning BEHALTEN (Null crasht per Assert, Neukalibrierung erst mit echten Chain-Daten in P2P-Phase)
- miner_tests folgt: Regtest-Fixture + CSV-Deferral via -testactivationheight (pré-CSV-Semantik wie frisches Mainnet), Nonce-Grinding ab Tabellen-Offset, Witness-Drop, wandnahe Blockzeiten (Min-Difficulty-Idleness)
- Tests: C++ 865/866 (0 Failures, 27.029.716 Assertions); funktional 10/10

## D-014 — ASERT-Anker: Genesis + virtueller Parent == Genesis-Zeit
- Problem: BCH-Norm verbietet Anker-Höhe 0 (kein Parent); neue Kette hat keine Historie
- Alternativen: (a) virtueller Parent T0-600 (permanenter +600s-Bias ≈ 0,24% leichter), (b) virtueller Parent == Genesis-Zeit (steady exakt)
- Gewählt: (b) — Steady-State reproduziert Anker-Target exakt (per Vektor bewiesen); Anker-Zeit/Bits aus Genesis gelesen (Single Source, keine Konstanten-Duplikate)
- Tests: 20 Differential-Vektoren C++==Python + Steady/Clamp/Property-Suite

## D-015 — ASERT-Overflow-Bounds (bewiesen, nicht geraten)
- Anker ≤ powLimit (< 2^224) × Faktor (< 2^18) ⇒ Produkt < 2^242 (kein Wrap)
- num_shifts ±512 Early-Outs sind sound (jenseits davon greift garantiert ein Clamp)
- int64-Exponenten: erreichbare Inputs (uint32-Zeiten, int32-Höhen) bleiben < 2^57; Halflife ≤ 0 fällt total auf max_bits zurück
- Trunc-Division (C++-/Python-trunc identisch), Floor-Shift ohne Shift-Semantik-Annahme formuliert

## D-016 — PermittedDifficultyTransition unter ASERT: wohlgeformt-statt-eng
- Problem: Bitcoin-2016-Transitionsregel lehnt legitime ASERT-Blöcke ab (Headersync-DoS-Filter)
- Gewählt: unter fUseASERT nur Wohlgeformtheit (≠0, kein Overflow/Negativ, ≤ powLimit); volle Regeln via CheckProofOfWork + Kontext-Validierung; Presync-Missbrauch bleibt durch Work-/Commitment-Bilanz + Redownload-PoW begrenzt (Threat-Modell!)
- pow_tests-2016-Vektoren laufen mit explizit Nicht-ASERT-Params (NoAsert-Helper) — keine Vektor-Manipulation

## D-017 — PQC-Produktions-Backend: liboqs 0.15 via vcpkg-Feature
- Problem: Echte ML-DSA-65 ohne eigene Krypto-Implementierung (§48: kein Library-Name als Beweis, aber auch keine DIY-Krypto)
- Gewählt: `pqc`-Feature (default-on) + `find_package(liboqs)` + `OQS::oqs`; HAVE_LIBOQS PUBLIC propagiert; Backend wirft BACKEND_MISSING ohne Lib; Größen gegen FIPS-204-Parameter gekreuzt; Version im Proof protokolliert
- Tests: Round-Trip + Negative + NIST-ACVP-KAT (extern/pur/leer-context) + Benchmarks

## D-018 — KAT-Reichweite ehrlich begrenzt
- 1 externer NIST-ACVP-sigVer-Vektor (mehr pure/empty-context gab das File nicht her) + abgeleitete Negative + Round-Trips + Größen-Kreuzcheck
- Kein FIPS-Konformitäts-Claim aus Tests allein; OQS warnt selbst vor Production-Use ohne Audit (RESEARCH); SLH-DSA-Backend + Wallet-Integration + Skript-Aktivierung = Folge-Phasen

## D-019 — P2PQ-Konsens: always-active, BIP143-SIGHASH_ALL, Kapselungs-Pattern
- Problem: v1/35B war anyone-can-spend-Fallback; P2PQ-Spends brauchen Durchsetzung + Sighash-Bindung + Amount-Zugang im Interpreter
- Gewählt: (a) always-active ab Genesis (keine Pre-Historie → kein Transitionsrisiko; dokumentiert inkl. Testnet-Replay-Nuance); (b) BIP143-Sighash mit scriptCode == P2PQ-spk (Präzedenz wie P2WPKH-Rekonstruktion) + SIGHASH_ALL fix v1 (kein Sighash-Byte im Witness-Format); (c) CheckPQCSignature als BaseSignatureChecker-Virtual (fail-closed default, Generic-Override mit tx/nIn/amount, Deferring-Forward); (d) Link-Richtung bitcoin_consensus → pqc (kein Zyklus)
- Tests: echter VerifyScript-e2e-Spend (liboqs-Signatur über echter Sighash) + 4 Negative (mutiert/falscher Amount/leer/kurz)
- Offen: Wallet-Keystore/RPC/GUI, Policy-Limits (Standardness), Mempool-Akzeptanz-Tests auf Regtest mit echten P2PQ-Outputs

## D-020 — p2pq()-Deskriptor: Adress-Layer jetzt, Custody später
- Problem: Wallet braucht P2PQ-Empfang/Tracking ohne ECDSA-Keystore-Umbau in einem Schritt
- Gewählt: `p2pq(<hex>)` als pubkey-tragender Deskriptor (Längen-Dispatch 1952/32, keine Ranges, TOP-only, IsSolvable=false wie addr(), OutputType BECH32M); FlatSigningProvider-PQC-Maps + SignStep-Hook + RPC-Send bewusst NÄCHSTE Phase (kein halbfertiges Custody)
- Tests: C++ parse/expand/round-trip/Negative + funktional wallet_pqc (echter Import + Funding + Tracking, solvable=false bewiesen)

## D-021 — PQC-Custody: Deskriptor-Container + dedizierte RPC (kein PSBT-Umbau)
- Problem: PQC-Keys (keine HD-Derivation, kein sk→pk) passen nicht in xpub/WIF-Schema; PSBT-Felder für Witness-Blobs fehlen
- Gewählt: (a) `p2pq(pub[,priv])`-Custody-Form (Längen-validiert, ToString leckt nie Privates, ExpandPrivate füllt pqc_keys); (b) FlatSigningProvider.pqc_keys + GetPQCKey/GetPQCPubKeys-Virtuals; (c) BaseSignatureCreator::CreatePQCSig-Virtual (Mutable echt mit BIP143/Amount, Dummy größenkorrekt für Fee-Schätzung); (d) SignStep/ProduceSignature-P2PQ-Ast; (e) `signpqcwithkey` + `createpqcaddress` RPCs (Offline-Signer-Muster wie signrawtransactionwithkey, fail-closed: Amount Pflicht, nur P2PQ-Inputs)
- Policy: mempool akzeptiert wohlgeformte P2PQ-Inputs (bounded Verifikation, keine unknown-script-DoS-Fläche); Standardness-Tuning folgt in Ökonomie-Phase
- Tests: C++ ProduceSignature→VerifyScript-Loop + RPC-e2e fund→sign→send→mine→confirm + 2 Negative (falscher Key, fehlender Amount)

## D-022 � Storage-Proof: PDP-lite Spot-Checks (kein PoRep), crypto-only DAG
- Problem: Speicher-Nachweis ohne Trusted Setup / SNARKs, deterministisch verifizierbar
- Gew�hlt: (a) 64KiB-Chunks, SHA256d-Leaves, Bitcoin-Duplicate-Odd-Merkle; (b) Challenge k=16 aus SHA256d(root||provider||BE64(epoch)); (c) Proof-Envelope v1 strikt/versioniert; (d) src/storage/ linkt NUR bitcoin_crypto (kein consensus � Hook sp�ter inward: consensus?storage, gleiche Regel wie pqc); (e) Merkle-�quivalenz zu consensus per Test bewiesen statt Code-Duplikation zu riskieren
- Ehrlichkeit: Spot-Checks beweisen Sample-Abrufbarkeit (Kurve in PROOFS/storage), KEIN Replikations-Nachweis, KEINE Voll-Replikations-Garantie; Registry/Rewards/Failure/Exit = Phase 14

## D-023 � Storage-RPC: eigene Kategorie + Raw-Order-Konvention
- Gew�hlt: (a) eigene RPC-Kategorie storage mit RegisterStorageRPCCommands (statt rawtransactions-Anhang � eigene Dom�ne, eigene Bounds); (b) Hashes als RAW-Byte-Order-Hex (bewusste Abweichung von der display-reversed RPC-Norm, begr�ndet + in Help dokumentiert); (c) storageverify fail-closed (valid=false+reason, wirft nie f�r Proof-Inhalt); (d) Bounds Content 1MiB / Samples 1024 / Pfad 64; (e) test/config.ini nur lokal (CMake-Build, Binary-Overrides via Env), NICHT committen
