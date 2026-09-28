# Network-Identity Kollisionsprüfung (DRAFT, 2026-09-28)

Quellen (§4, je 1 offiziell + 1 unabhängig):
- https://bitcoindevelopers.org/docs/reference/p2p-network-ref/ (BTC main/test/regtest Magics+Ports)
- https://github.com/dan-da/coinparams/blob/HEAD/coinnetworking.md (BCH/LTC/DOGE/DASH-Magics+Ports)
- http://zips.z.cash/zip-0204 (ZEC-Magics+Ports)
- Lokal: `src/kernel/chainparams.cpp` (BTC testnet4/signet/regtest, verifiziert per Grep)

Bekannte belegte Magics: f9beb4d9, 0b110907, fabfb5da, fbc0b6db, c0c0c0c0,
e3e1f3e8, bf0c6bbd, 24e92764, fa1af9bf, aae83f5f, 1c163f28.
Bekannte belegte P2P-Ports: 8333, 9333, 22556, 9999, 8233, 18333, 18444, 38333, 48333.

## NUMS-Vorschlag (DRAFT, NICHT eingefroren, §259)

`magic = SHA256("<label>")[:4]` — Nothing-Up-My-Sleeve, reproduzierbar:

| Netz | Label | Magic | P2P-Port (DRAFT) | RPC-Port (DRAFT) |
|---|---|---|---|---|
| mainnet | QuantBTC-mainnet-v1 | `3c 59 66 52` | 8444 | 8442 |
| testnet | QuantBTC-testnet-v1 | `c8 9d 07 20` | 28444 | 28442 |
| regtest | QuantBTC-regtest-v1 | `c6 1e 7b 95` | 28445 | 28443 |
| signet | QuantBTC-signet-v1 | `47 94 45 e6` | 38445 | 38442 |

Keine Kollision mit obiger Tabelle. Ports NICHT gegen vollständige IANA-/Altcoin-Liste geprüft → Test `test_quantbtc_magic` + `test_cross_network_magic_rejected` (§26) Pflicht vor Freeze.
Bech32-HRP DRAFT: main `qb`, test `tqb`, regtest `qbrt` (Kollisionsfreiheit gegen BIP173-Registry: NOT VERIFIED — Test Pflicht).
Base58 DRAFT: P2PKH 58 (Ziel: `Q`-Präfix), Script 55 — Erstzeichen-Behauptung NOT VERIFIED, Test Pflicht.
Datadir: `QuantBTC` / `~/.quantbtc/` (§28). Binaries: quantbtcd/cli/tx/qt (§104).
chain_id DRAFT: `QBTC-1` (protocol_version 1).
