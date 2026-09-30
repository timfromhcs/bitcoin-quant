# QuantBTC

QuantBTC ist ein experimenteller Bitcoin-Core-Fork mit eigener Kette, Post-Quantum-Kryptografie
und einer Storage-Ökonomie. **Experimentell: keine Audits, kein Mainnet mit realem Wert.**
Aktiver Entwicklungsbranch ist `feat/chain-identity`.

Herkunft: Bitcoin Core (Upstream-Stand `05bc2f53ce`, Baseline per `git bundle` gesichert).
Lizenz: MIT, siehe [COPYING](COPYING).

## Ketten-Identität (eigene Kette, kein Bitcoin)

| Netz    | Magic      | P2P-Port | RPC-Port | Adress-HRP |
|---------|------------|----------|----------|------------|
| Mainnet | `3c596652` | 8444     | 8442     | `qb`       |
| Testnet | `c89d0720` | 28444    | 28442    | `tqb`      |

Weitere Netze (Signet/Regtest/Testnet4) haben eigene Magics/Ports, Details in
`src/kernel/chainparams.cpp`. Datenverzeichnis: `QuantBTC` bzw. `~/.quantbtc/`.
Base58-Versionsbytes (Mainnet): Pubkey `58`, Script `120`. Chain-ID: `QBTC-1`.

Genesis-Block (eingefroren v1, `contrib/quantbtc/genesis/` reproduzierbar):

- Hash: `000000002f24a967129873ad204d29f947f0452710c72c9aacf45fcf2d8f2881`
- Nonce `887863234`, Time `1758931200`, nBits `0x1d00ffff`
- Coinbase: `bab4d3bab87e3ca9493b99d64f7a4db66a9b064ec7128225da032e9bdef2f9c1`

Konsens: SHA256d-PoW, ASERT-Difficulty (`aserti3-2d`), historische Bitcoin-Upgrades
(Buried Deployments) ab Höhe 1 isoliert. Details: `docs/protocol/quantbtc-spec-v1.md`.

## Implementierter Stand

- **PQC-Transaktionen:** ML-DSA-65 via liboqs 0.15 (Produktions-Backend, NIST-ACVP-KAT
  geprüft), Witness-Programm v1/35B, Bech32m-Adressen, `p2pq()`-Deskriptoren,
  Wallet-Custody, RPCs `createpqcaddress` / `signpqcwithkey` (Ende-zu-Ende auf Regtest
  bewiesen: fund → sign → send → mine → confirm).
- **Storage-Proofs (PDP-lite):** 64-KiB-Chunks, SHA256d-Merkle (Bitcoin-Semantik),
  deterministische Challenges `SHA256d(root || provider || epoch)`, k=16 Samples,
  versionierte Proof-Envelope. RPCs `storagecommit` / `storagechallenge` /
  `storageprove` / `storageverify` (fail-closed, funktional getestet).
- **Storage-Ökonomie (Kern):** ProviderRecord-Codec, exakte Fixpunkt-Rewards
  `floor(verified_bytes × price / 2³⁰)` (portable 128-Bit-Math, kein Float),
  Status-Maschine ACTIVE ↔ SUSPENDED → EXITED (terminal).
- **NFT (Mint-Record, Kern):** Record-Codec + deterministische
  `nft_id = SHA256d(collection_id || BE64(serial))`. C++-Tests ausstehend (siehe Blocker).
- **Offen / ehrliche Lücken:** Provider-Registry, Reward-Payouts, Konsens-Hooks,
  P2P-Challenge-Gossip, NFT-Transfers/Lifecycle, State-Packs. Remote-CI läuft nicht
  (externer Blocker). Details in `.agent/BLOCKERS.md`, Entscheidungen in
  `.agent/DECISIONS.md`, Nachweise in `.agent/PROOFS/`.

Protokoll-Spec: [`docs/protocol/quantbtc-spec-v1.md`](docs/protocol/quantbtc-spec-v1.md).
Masterplan: `GEMINI.md`. Arbeitsstand: `.agent/STATE.md`.

## Bauen (getesteter Pfad: Windows + vcpkg)

Voraussetzungen: Visual Studio Build Tools, vcpkg (`C:/vcpkg`, Triplet `x64-windows`).
liboqs 0.15 wird per `find_package` gesucht und aktiviert das PQC-Produktions-Backend;
ohne Fund baut nur das deterministische Test-Backend (Configure-Log beachten).

```powershell
cmake -S . -B C:\btc-quant\build `
  "-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  "-DVCPKG_TARGET_TRIPLET=x64-windows" `
  -DBUILD_GUI=ON -DBUILD_TESTS=ON -DBUILD_BENCH=ON -DWITH_ZMQ=ON
cmake --build C:\btc-quant\build --config Debug --target bitcoind
```

Binaries landen in `<build>\bin\Debug\` (`bitcoind`, `bitcoin-cli`, `bitcoin-qt`,
`test_bitcoin`, …). Linux: Standard-Bitcoin-Core-Weg (`cmake -B build && cmake --build build`,
Abhängigkeiten per `doc/build-unix.md`), hier aber nicht verifiziert.

## Tests

C++-Suite (Boost, verifizierter Stand: **887 Fälle, 0 Failures**, ~27 Mio. Assertions):

```powershell
.\build\bin\Debug\test_bitcoin.exe --report_level=short --report_sink=stdout
```

Funktionale Tests (echte Nodes, z. B. `test/functional/rpc_storage.py`,
`test/functional/wallet_pqc.py`) brauchen `test/config.ini` (lokal, nicht committet):

```ini
[environment]
SRCDIR=C:\btc-quant\bitcoin-quant
BUILDDIR=C:\btc-quant\build
EXEEXT=.exe
CLIENT_BUGREPORT=https://github.com/timfromhcs/bitcoin-quant/issues
```

Bei Multi-Config-Builds (Visual Studio) zusätzlich Binary-Overrides setzen, da die
Binaries unter `<build>\bin\Debug\` liegen:

```powershell
$env:BITCOIND="C:\btc-quant\build\bin\Debug\bitcoind.exe"
$env:BITCOINCLI="C:\btc-quant\build\bin\Debug\bitcoin-cli.exe"
$env:BITCOINUTIL="C:\btc-quant\build\bin\Debug\bitcoin-util.exe"
$env:BITCOINTX="C:\btc-quant\build\bin\Debug\bitcoin-tx.exe"
$env:BITCOINWALLET="C:\btc-quant\build\bin\Debug\bitcoin-wallet.exe"
$env:BITCOIN_BIN="C:\btc-quant\build\bin\Debug\bitcoin.exe"
python test\functional\rpc_storage.py
```

Python-Referenzen (Spec-Orakel, Standardbibliothek, alle mit Selbsttest):

```powershell
python contrib\quantbtc\ref\storage_proof.py
python contrib\quantbtc\ref\storage_econ.py
python contrib\quantbtc\ref\nft_record.py
python contrib\quantbtc\genesis\generate_genesis.py  # Genesis-Reproduktion, 2x-Lauf
```

C++-Vektoren werden aus den Referenzen generiert (`contrib/quantbtc/gen_*.py`) und sind
als `src/test/*_vectors.h` eingecheckt (Differential-Tests C++ == Python).

## Projekt-Konventionen

Jede Protokoll-Änderung braucht: Spec-Eintrag, Python-Referenz mit Selbsttest,
C++-Port mit Differential-Test, Nachweis unter `.agent/PROOFS/`, Entscheidungs-Eintrag
unter `.agent/DECISIONS.md`. Was nicht grün gelaufen ist, wird als NOT RUN markiert —
niemals Grün vortäuschen. Reine Mathe-/Codec-Layer linken nur `bitcoin_crypto`, nie
`bitcoin_consensus` (Konsens ruft hinein, nie umgekehrt).
