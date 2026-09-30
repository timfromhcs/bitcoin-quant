# PROOFS — NFT mint-record core (§70–§77, Phase 13 Kern)

## Status: TEIL-BEWIESEN (B-004)
- **Python-Referenz: PASS (2x)** — `contrib/quantbtc/ref/nft_record.py`:
  Codec Round-Trip (1 B / 25 B / 10000 B Scripts, Serial/Epoche 0/7/2^64-1),
  nft_id-Determinismus + Domain-Separation, 3 Negative
- **Vektoren: 4, byte-identisch regeneriert** — Generator-Rekonstruktion nach
  E:-Ausfall via Re-Generierung als **byte-identisch BEWIESEN**
  (`C:\ft\salvage\` vs. Klon-Output)
- **C++ (`src/nft/`, `nft_tests`): NOT RUN** — Code geschrieben + verdrahtet
  (CMake-Wiring byte-identisch zum E:-Stand verifiziert), aber Kompilierung
  durch Laufwerksausfall verhindert. KEIN Grün-Vortäuschen: Tests nachholen,
  sobald C:-Build steht.

## Design (ehrlich begrenzt)
- MintRecord v1 + nft_id = SHA256d(collection_id || BE64(serial));
  Eindeutigkeit = Registry-Layer, nicht Formel
- Gaps: Transfers, Registry, Collections, Royalties, Konsens-Hook, Lifecycle
