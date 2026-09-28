# NETWORK_IDENTITY_MAP — Baseline (2026-09-28, verifiziert per Grep an src/kernel/chainparams.cpp)

Primärdatei: `src/kernel/chainparams.cpp` (745 Zeilen). Alle Werte per `rg` verifiziert.

## Genesis (assert-Zeilen)

| Chain | Block-Zeile | Genesis-Hash | Labels |
|---|---|---|---|
| main | :158–161 | `000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f` | ts 1231006505, nonce 2083236893, bits 0x1d00ffff; Merkle `4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b`; Times 03/Jan/2009 |
| testnet3 | :284–287 | `000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943` | ts 1296688602, nonce 414098458 |
| testnet4 | :394–403 | `00000000da84f2bafbbc53dee25a72ae507ff4914b867c565be350b0da8bf043` | ts 1714777860, nonce 393743547; eigener Timestamp 03/May/2024 |
| signet | :537–540 | `00000008819873e925422c1ff0f99f7cc9bbb232af63a077a480a3633bee1ef6` | ts 1598918400, nonce 52613770, bits 0x1e0377ae |
| regtest | :631–634 | `0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206` | ts 1296688602, nonce 2, bits 0x207fffff |

## Netzwerk-Magics (pchMessageStart)

| Chain | Zeilen | Bytes |
|---|---|---|
| main | :149–152 | `f9 be b4 d9` (F9BEB4D9) |
| testnet3 | :275–278 | `0b 11 09 07` |
| testnet4 | :383–386 | `1c 16 3f 28` |
| signet | :532 | dynamisch aus Challenge (`src/kernel/signet.h:27-35`) |
| regtest | :620–623 | `fa bf b5 da` |

## Ports

P2P: main 8333 (:153), testnet3 18333 (:279), testnet4 48333 (:387), signet 38333 (:534), regtest 18444 (:624).
RPC (`src/chainparamsbase.cpp:61-76`): 8332 / 18332 / 48332 / 38332 / 18443. Tor-Incoming-Doku: 8334/18334/38334/48334/18445.

## DNS-Seeds (`src/kernel/chainparams.cpp` vSeeds)

- main (:168–174): bluematt, jonasschnelli, petertodd, sprovoost, emzy, wiz, achownodes
- testnet3 (:292–295), testnet4 (:408–409), signet (:475–476), regtest: `dummySeed.invalid.` (:638)

## Adress-Präfixe

- main (:176–183): base58 0x00/0x05/0x80, xpub `0488B21E`, xprv `0488ADE4`, bech32 `bc`, silent `sp`
- testnet3/4/signet/regtest: 111/196/239, tpub/tprv, bech32 `tb` (regtest `bcrt`), silent `tsp` (regtest `sprt`)

## AssumeValid / MinChainWork / Snapshots

- main (:141–142): minWork `…145ec036acc5ba740052af1a0`, assumeValid `…748969ec33043c0e52a763c6dd5193861f559f2c72e3` (Block 966143); Snapshots Höhen 840000/880000/910000/935000/965000
- testnet3/testnet4/signet: Werte siehe Grep-Protokoll; regtest: beide `uint256{}`

## Klassifikation (§19)

Alle Treffer = UPSTREAM (Bitcoin Mainnet-Identität aktiv). Für QuantBTC: CHANGE erforderlich in `src/kernel/chainparams.cpp`, `src/chainparamsbase.cpp`, `src/common/args.cpp:859-889` (DataDir), `CMakeLists.txt:29`, `src/qt/guiconstants.h:49-55`, `share/setup.nsi.in`. Werte NICHT einfrieren bis Protokolldesign fertig (§259).
