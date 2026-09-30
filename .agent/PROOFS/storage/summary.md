# PROOFS — Storage proof core (§78–§81, Phase 11 Kern)

## Was bewiesen ist
- **Differential Python→C++ (§114):** 3 Vektoren (1 B / 192 KiB+17 / 100 KiB),
  Root + 16 Challenge-Indizes + Proof + Envelope **byte-identisch**
  (`src/test/storage_test_vectors.h`, generiert via
  `contrib/quantbtc/gen_storage_vectors.py`)
- **Single-source (§157):** `storage::MerkleRoot == consensus ComputeMerkleRoot`
  auf gleichen Leaves (kein Merkle-Fork)
- **Negatives:** chunk-tamper / path-tamper / falsche Root → ROOT_MISMATCH,
  leerer Chunk → BAD_CHUNK_SIZE, gekürzter Pfad → PATH_LENGTH_MISMATCH/ROOT_MISMATCH
- **Envelope (§207):** versioniert (v1), strikt — Trail-Byte/Trunkierung/
  Versions-Bump/leer werden abgewiesen; Encode deterministisch
- **Suite:** `storage_tests` 4/4, 111/111 Assertions
  (`quantbtc-backup/09-tests/storage-tests.log`); Gesamtsuite jetzt **882**

## Detection-Kurve (k=16, Spot-Check-Ehrlichkeit)
- Provider löscht p-Anteil: P(Entdeckung) = 1-(1-p)^16
- p=1% → 0.1485 | p=5% → 0.5599 | p=10% → 0.8147 | p=50% → ~1.0000
- Ehrlich dokumentiert: beweist Abrufbarkeit der Samples, kein PoRep
  (Replikations-Nachweis), keine Voll-Replikations-Garantie

## Offene Stufen (NICHT in diesem Commit)
- Provider-Registry, Reward-Formel (fixpunkt), Failure/Exit/Sybil → Phase 14
- Konsens-Hook (consensus→storage), P2P-Challenge-Gossip → Phase 16/14
- RPC (`storageprove`, `storageverify`) → Phase 15

## RPC-Evidenz (2026-09-30)
- test/functional/rpc_storage.py PASS auf echter Node (unabh�ngige hashlib-Re-Implementierung als Orakel)
- commit/challenge/prove/verify �ber 5 Content-Gr��en (1 B..192 KiB) + Epochen 0/7, alle Negative fail-closed, alle Validierungsfehler -8
- Log: quantbtc-backup/09-tests/rpc-storage-func.log

## Economy-Evidenz (2026-09-30)
- Differential Python->C++: 21 Reward-Vektoren (2^64-Limb-Stress, Floor-Kanten) + 4 Record-Vektoren byte-identisch
- storage_econ_tests 5/5, 747 Assertions; mit Proof-Suite 9/9, 858 Assertions (Log: backup 09-tests/storage-econ-tests.log); Gesamtsuite 887
- Bugs gefunden+behoben: Shift-Maske (>>30-Faltung exakt bewiesen), OVERFLOW-Makro-Kollision, 2x Off-by-one in Negativ-Offsets
