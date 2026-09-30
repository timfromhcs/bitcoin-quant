# QuantBTC Protocol Specification v1 (SKELETON — §22, Stand 2026-09-28)

> Status: DRAFT. Kein Wert eingefroren. Alle Felder = TBD bis Difficulty-/PQC-/Economy-Evidenz vorliegt (§39–§42, §48–§49).

## 1. Chain ID
- `chain_id`: TBD (eindeutig, nicht mit Bitcoin-Netzen kollidierend)

## 2. Netzwerk-Kennung (DRAFT-Werte, NICHT eingefroren — Evidenz: `.agent/RESEARCH/network-identity-collisions.md`)
| Netz | Magic (NUMS) | P2P-Port | RPC-Port | Seeds | Status |
|---|---|---|---|---|---|
| mainnet | `3c 59 66 52` | 8444 | 8442 | TBD (eigene Seeds) | DRAFT |
| testnet | `c8 9d 07 20` | 28444 | 28442 | TBD | DRAFT |
| regtest | `c6 1e 7b 95` | 28445 | 28443 | — | DRAFT |
| devnet/signet | `47 94 45 e6` | 38445 | 38442 | TBD | DRAFT |

chain_id DRAFT: `QBTC-1`. Adressen DRAFT: Main Bech32 `qb`/Base58 P2PKH 58 (`Q`)/P2SH 120 (`q`); Testnet `tqb`/112/196; Regtest `qbrt`/`qsprt`. EXT-Key-Versionen bleiben BIP32-kompatibel (D-008). Datadir: `QuantBTC`/`~/.quantbtc/`. Silent-Payments-HRP: `qsp`/`tqsp`/`qsprt`.

Aktuell aktiv (Bitcoin-Upstream, zu ERSETZEN): siehe `.agent/AUDITS/NETWORK_IDENTITY_MAP.md`.

## 3. Genesis (§30–§32) — EINGEFROREN v1 (2026-09-28)
- Generator: `contrib/quantbtc/genesis/generate_genesis.py` (deterministisch, NUMS-Key, Chunk-Ledger-Mining)
- timestamp 1758931200 (roll 0), version 1, nBits 0x1d00ffff, nonce 887863234, reward 50 QBTC
- tx/merkle: `bab4d3bab87e3ca9493b99d64f7a4db66a9b064ec7128225da032e9bdef2f9c1`
- genesis: `000000002f24a967129873ad204d29f947f0452710c72c9aacf45fcf2d8f2881`
- Manifest: `contrib/quantbtc/genesis/generated/genesis-manifest.json`; Evidenz: `.agent/PROOFS/genesis/`
- Offen: Clean-Checkout-Build-Verifikation (ausstehend)

## 4. Block-Header / PoW (§36–§38)
- Primitiv: SHA256d (`SHA256(SHA256(header))` vs. Kompakt-Target) — unverändert
- Ehrliche Sprache: KEIN "quantum-proof"/"zero quantum advantage" für SHA256d; PQ-Sicherheit gilt Transaktions-Autorisierung (§236-Wortlaut bindend)

## 5. Difficulty (§39–§42) — ASERT IMPLEMENTIERT (aserti3-2d)
- Halbwertszeit 172800 s, Spacing 600 s; Anker = Genesis (Höhe 0, Parent-Zeit
  == Genesis-Zeit per D-014, Bits == Genesis-nBits)
- Integer-exakte Implementierung (`CalculateASERT`), Differential-Referenz
  `contrib/quantbtc/ref/asert.py`, 20 Vektoren in `genesis/vectors/asert-vectors.json`
- Testnets behalten Bitcoin-Retarget; RSP: kein Zero-Quantum-Claim (s. §4)

## 6. Konsens-Isolation (§33–§35) — UMGESETZT
- `defaultAssumeValid`: leer (kein Bitcoin-Wert)
- `AssumeUTXO`: keine Snapshots ohne echte QuantBTC-Historie
- minChainWork leer; TxStats null; Bitcoin-Genesis ersetzt (s. §3)
- Alle Buried-Upgrades (BIP34/65/66/CSV/Segwit) ab Höhe 1 aktiv; keine Script-Exceptions
- Headersync-Tuning vorerst übernommen (Neukalibrierung in P2P-Phase)

## 7. PQ-Transaktionspfad (§46–§55) — FORMAT + BACKENDS STEHEN, Wallet/Aktivierung folgt
- Abstraktion: PQCAlgorithm/KeyPair/Signature/Verifier/Registry; Backend-Schichten Referenz/Produktion/Test
- Produktions-Backend: liboqs 0.15.0, ML-DSA-65 (FIPS 204, Größen 1952/4032/3309); SLH-DSA-SHA2-128s reserviert
- Algorithm-IDs: 0x0000 ungültig; 0x0001 ML-DSA-65; 0x0002 SLH-DSA-SHA2-128s (reserviert); 0xFFFF TEST_ONLY (niemals konsens-gültig, per Test bewiesen)
- PQC_VERSION = 1 (1 Byte). Alle konsens-sichtbaren Blobs sind versionierte Envelopes:
  `pubkey_blob = ver(1) || alg_id(2, LE) || raw_pubkey`
  `sig_blob    = ver(1) || alg(2, LE) || raw_sig`
- Deskriptor `p2pq(<hex>)` (pubkey-tragend, watch/receive; Längen-Dispatch, keine Ranges, TOP-only)
- P2PQ scriptPubKey: `OP_1 <0x23 <ver(1) || alg(2 LE) || keyid(32)>>`,
  mit `keyid = SHA256(raw_pubkey)` (32 Bytes, voller Hash, keine Trunkierung)
- P2PQ-Spend: Witness `[sig_blob, pubkey_blob]`; Validierung: Versionen == Script-Version,
  Algorithmen gleich, Größen == Algorithmus-Parameter, `SHA256(raw_pubkey) == keyid`,
  Signatur über BIP143-Sighash (SIGHASH_ALL fix v1) gültig; Konsens-Ast im Interpreter
  aktiv ab Genesis (D-019); Wallet-Integration (Keystore/RPC) folgt
- Benchmarks/Bibliotheks-KAT vor Backend-Freeze Pflicht (§49)

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
