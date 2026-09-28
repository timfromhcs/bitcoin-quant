# PLAN.md — Ableitung aus GEMINI.md (§24, §256–§260)

Reihenfolge (verbindlich): 0 Backup → 1 Audit → 2 Baseline-Build → 3 Protokoll-Spec → 4 Chain-Identity → 5 Genesis → 6 Consensus-Isolation → 7 SHA256d-PoW → 8 Difficulty → 9 PQC → 10 Wallet → 11 Storage → 12 State-Packs → 13 NFT → 14 Storage-Economy → 15 RPC/API → 16 P2P → 17 Qt-UX → 18 Cross-Chain → 19 Testsystem → 20 Security-Hardening → 21 Testnet → 22 Chaos/Long-Run → 23 RC → 24 Genesis-Freeze → 25 Repro-Release → 26 Mainnet.

Erste Implementation (nach Spec-Skeleton): Chain-Identity-Deliverable (§259) — chain_id, network magic, ports, datadir, Adress-Präfix-Platzhalter, Seed-Strategie, Binary-Naming-Strategie, chainparams-Isolation, Tests, Doku. Werte NICHT einfrieren bis Protokolldesign fertig.

Stop-Bedingungen (§268): Genesis mehrdeutig, Konsens-Divergenz, PQ-Verifikations-Divergenz, State-Root-Divergenz nach Dekompression, Wallet-Restore-Verlust, Reward-Overflow, unverifizierbare Security-Dependency, nicht reproduzierbarer Release-Build, Secret-Fund, widersprüchliche Tests, Remote-CI-Divergenz → STOP + Evidence sichern.
