# RISKS.md — 2026-09-28

- R-001: Baseline baut ohne vcpkg-Dependencies nicht → alle Build-/Test-Gates BLOCKED bis B-001 gelöst
- R-002: Volle Bitcoin-Builds auf Windows dauern Stunden + brauchen ~100 GB Deps/Artefakte → Zeit-/Platten-Budget einplanen
- R-003: Token wurde im Chat im Klartext übergeben → Kompromittierungs-Risiko; Empfehlung: nach Session rotieren (noch NICHT geschehen — User-Entscheidung)
- R-004: Gesamt-Scope (273 Plan-Abschnitte bis Mainnet) übersteigt eine Session bei weitem → strikt phasenweise mit Checkpoints arbeiten (§227: keine Phase ohne Tests+Proofs+Checkpoint schließen)
- R-005: SHA256d bleibt Mining-Primitiv → ehrliche Quanten-Sprache Pflicht (§2, §38, §236); keine "quantum-proof"-Claims
