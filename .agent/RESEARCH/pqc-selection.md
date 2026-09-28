# PQC-Auswahl Evidenz (DRAFT, 2026-09-28)

Quellen (§4: NIST-Standard + Implementierungs-Tracker):
- FIPS 204 (ML-DSA), final 2024-08-13: https://csrc.nist.gov/pubs/fips/204/final
- FIPS 205 (SLH-DSA), final 2024-08-13: https://csrc.nist.gov/pubs/fips/205/final
- liboqs ML-DSA (FIPS-204-final) PR #1919, gemergt 2024-11-26 (Release 0.12.0)
- liboqs SLH-DSA-Integration Issue #1894 (FIPS-205-Anpassungen, u.a. message_to_indices vs. SPHINCS+ 3.1 — NICHT binärkompatibel zum alten Stand)

## Kandidaten (§48)

| Algorithmus | Standard | Status Implementierung | Bemerkung |
|---|---|---|---|
| ML-DSA-65 (empfohlen primär) | FIPS 204 | liboqs ≥0.12.0 aus pq-crystals-Upstream | balanciert: pk 1952 B, sig 3309 B |
| SLH-DSA-SHA2-128s (Backup) | FIPS 205 | liboqs-Integration fortgeschritten, Stand VERIFIZIEREN | konservativ (hashbasiert), große Sigs (~7856 B), langsam |
| ML-DSA-44 / -87 | FIPS 204 | dto. ML-DSA | -44 kleiner/schwächeres NIST-Level, -87 größer; Benchmark entscheidet |

## Offene Verifikation (§49, vor Freeze Pflicht)
- liboqs-Build unter Windows/MSVC + Version pinnen, Lizenz prüfen (MIT, aber VERIFIZIEREN)
- KAT-Testvektoren aus FIPS-Publikationen gegen liboqs laufen lassen
- Benchmarks (keygen/sign/verify, Größen, Block-Validierungs-Impact) auf Zielplattformen
- Keine Produktions-Claims bis dahin (OQS selbst warnt vor Production-Use ohne Audit)

Empfehlung: PQC-Abstraktion (§46) JETZT bauen (algo-agil), Backend-Auswahl nach Benchmarks einfrieren.
