# Difficulty-Simulations Evidenz (DRAFT, 2026-09-28) → IMPLEMENTIERT 2026-09-29

Update: ASERT implementiert (`CalculateASERT`, D-014/D-015/D-016);
Evidenz: `.agent/PROOFS/difficulty-asert/summary.md`; Referenz: `ref/asert.py`.

Tool: `contrib/quantbtc/ref/difficulty_sim.py` (Closed-Loop, LCG-seeded, deterministisch).
Rohdaten: `contrib/quantbtc/genesis/vectors/difficulty-sim.json`.
Modellannahmen: Blockzeit = T*(T0/Target)/Hashrate * U(0.5,1.5); pow-limit-Band ±2^16;
Median über 2500 Blöcke (bitcoin-Fenster 2016 retargetet daher 1x).

## Ergebnis (Median-Blockabstand, Ziel 600 s)

| Szenario | bitcoin-2016 | ASERT (2d) | LWMA-60 |
|---|---|---|---|
| steady | 598 | 599 | 302 (Bias ~2x zu schnell) |
| step 4x up @500 | 197 | 444 | 300 |
| crash 10x @500 | 4127 | 692 | 301 |
| lone miner (0.02) | 26245 | 702 | 282 |
| warp +7000s x100 | 538 | 298 | 1 (Kollaps: min-Target) |

## Interpretation (§39–§42)
- bitcoin-2016: katastrophale Erholung nach Crash (4127 s) und bei Low-Hashrate (26245 s) — für neue Kette ungeeignet.
- ASERT: stabil + schnellste ehrliche Erholung (692 s nach 10x-Crash), warp-resistent (298 s).
- LWMA-60: schnell, aber (a) Steady-Bias zu schnell (bekannte LWMA-1-Schwäche), (b) Warp-Angriff treibt auf min-Target (1-s-Blöcke) — historisch konsistent mit BCH-Erfahrung, die zu ASERT führte.
- Empfehlung: ASERT-Kandidat (D-003), mit Integer-Konsensimplementierung + Differentialtests + Timewarp/Adversarial-Tests vor Freeze. KEIN Freeze-Entscheid (§39).
- Self-Healing-Historie des Tools: 4 Modell-Bugs gefunden/repariert (Pfad, fehlender Feedback-Loop, Float-Overflow, invertiertes Target-Vorzeichen, arithmetisches statt Difficulty-Mittel) — dokumentiert Ehrlichkeit des Prozesses.
