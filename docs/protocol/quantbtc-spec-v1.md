# QuantBTC Protocol Specification v1 (SKELETON — §22, Stand 2026-09-28)

> Status: DRAFT. Kein Wert eingefroren. Alle Felder = TBD bis Difficulty-/PQC-/Economy-Evidenz vorliegt (§39–§42, §48–§49).

## 1. Chain ID
- `chain_id`: TBD (eindeutig, nicht mit Bitcoin-Netzen kollidierend)

## 2. Netzwerk-Kennung
| Netz | Magic | P2P-Port | RPC-Port | Seeds | Status |
|---|---|---|---|---|---|
| mainnet | TBD (Kollisionsprüfung Pflicht, §26) | TBD (§27) | TBD | TBD (eigene Seeds, §25) | TBD |
| testnet | TBD | TBD | TBD | TBD | TBD |
| regtest | TBD | TBD | TBD | — | TBD |
| devnet | TBD | TBD | TBD | — | TBD |

Aktuell aktiv (Bitcoin-Upstream, zu ERSETZEN): siehe `.agent/AUDITS/NETWORK_IDENTITY_MAP.md`.

## 3. Genesis (§30–§32)
- Generator: `contrib/quantbtc/genesis/generate_genesis.py` (AUSSTEHEND — deterministisch, Spec+Manifest+Testvektor)
- `genesis-manifest.json`-Felder: chain_id, protocol_version, timestamp, version, nBits, nonce, reward, tx-hash, merkle-root, genesis-hash, tool-version, source-commit
- Freeze-Protokoll §31: doppelte unabhängige Generierung + Clean-Build + Genesis-Testsuite + Bitcoin-Genesis-Reject + externes Backup

## 4. Block-Header / PoW (§36–§38)
- Primitiv: SHA256d (`SHA256(SHA256(header))` vs. Kompakt-Target) — unverändert
- Ehrliche Sprache: KEIN "quantum-proof"/"zero quantum advantage" für SHA256d; PQ-Sicherheit gilt Transaktions-Autorisierung (§236-Wortlaut bindend)

## 5. Difficulty (§39–§42)
- Kandidaten: ASERT / LWMA / bounded EMA-Hybrid — Auswahl erst nach Simulation (§41: 1/2/5/10/100 Miner + Hashrate-Crash)
- Target-Spacing: TBD nach Messung (Propagation, Orphan-Rate, CPU, Bandbreite, Reorg, UX, Storage-Wachstum)

## 6. Konsens-Isolation (§33–§35)
- `defaultAssumeValid`: KEIN Bitcoin-Wert übernehmen (neue Kette → leer/QuantBTC-eigen)
- `AssumeUTXO`: erst nach echter QuantBTC-Historie + verifiziertem Snapshot-Format
- Bitcoin-spezifische minChainWork/TxStats/Genesis/Aktivierungen: entfernen/ersetzen

## 7. PQ-Transaktionspfad (§46–§55)
- Abstraktion: PQCAlgorithm/KeyPair/Signature/Verifier/Registry; Backend-Schichten Referenz/Produktion/Test
- Kandidaten: ML-DSA, SLH-DSA (Auswahl nach NIST-Spec + Build-/Lizenz-/Vektor-/Benchmark-Evidenz)
- Format: versionierter P2PQ-Output (version, algorithm_id, key-commitment); Krypto-Agilität Pflicht
- Testvektoren: gültig, mutierte Message/Signatur, falscher Key/Algo/Version, trunkiert/übergroß/malformiert

## 8. Wallet (§53–§56)
- Generierung/Derivation/Sign/Verify/Backup/Restore/Rescan/Display/Migration; keine Secrets in Logs; Locking/Zeroisierung; First-Start-Wizard (Wallet/Node-Only/Pruned/Storage/Advanced)

## 9. Node-Modi (§58–§62)
- Full/Pruned/Archive/Storage/Hybrid/Mining/Wallet; Hardware-aware Defaults (KEIN dynamischer Konsens); Pruning/Index-Kompatibilitätsmatrix mit klarem Fail statt silent

## 10. State-Packs (§63–§69)
- Format `.qsp` v1: HEADER/MANIFEST/INDEX/CHUNKS/FOOTER; logisch→deterministisch→chunked→komprimiert→Manifest+Hashes
- Verlustfrei: `state_root_before == state_root_after` + Record-Counts; Fallback auf Normalsync

## 11. NFT (§70–§77, §86)
- Trennung Ownership/Metadaten/Content-Commitment/Storage-Vertrag vs. Content-Bytes; Kern-Record (asset_id … status); Lifecycle MINTED→…→BURNED; bounded Budgets/Renewals; Grace statt Sofort-Löschung; ehrliche Lösch-Sprache (expired/unpinned/…)

## 12. Storage (§78–§85)
- Provider-Registrierung, Challenge→Proof→Verify→Reward (deterministisch), Challenge-Entropie nicht provider-kontrolliert, Sybil-Maßnahmen, Failure-/Exit-Pfade, Trennung Pruning vs. Content-Storage

## 13. Storage-Economy (§87–§88)
- Formel + Rundung + Overflow + Maxima + Transitionstests; Geldpolitik (Supply/Reward/Halving/Fees/Burn) vor Mainnet einfrieren

## 14. RPC/API (§89–§93)
- Module `quant/pq/nft/storage/snapshot`, Schemaversionierung, Capability-Discovery, Fehler-Modell (Code + Technik- + User-Message + Action)

## 15. P2P (§107–§108)
- Capability-Negotiation (versioniert), Resilienztests (Churn/Seeds/DNS/Slow/Malicious/Partition/Reconnect/Invalid)

## 16. Aktivierungs-Strategie
- TBD (BIP9/Height-Stubs aus Upstream entfernen, QuantBTC-eigen definieren)
