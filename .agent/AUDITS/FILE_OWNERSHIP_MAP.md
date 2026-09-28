# FILE_OWNERSHIP_MAP — Baseline (2026-09-28)

| Domäne | Datei/Modul |
|---|---|
| Chainparams | `src/chainparams.{cpp,h}`, `src/chainparamsbase.{cpp,h}`, `src/chainparamsseeds.h`, `src/kernel/chainparams.{cpp,h}` (Genesis-Klassen hier), `src/consensus/params.h`, `src/deploymentinfo.*`, `src/versionbits.*` |
| Konsens | `src/consensus/tx_check.*, tx_verify.*, merkle.*, consensus.h, amount.h, params.h` + `src/validation.*` + `src/kernel/checks.*` + `src/primitives/{block,transaction}.*` |
| PoW/Difficulty | `src/pow.*` (`GetNextWorkRequired`) + `src/arith_uint256.*` + `src/chain.*` + `src/kernel/chain.*` + `src/node/miner.*`, `mini_miner.*`, `block_template_manager.*` |
| Wallet | `src/wallet/{wallet,spend,receive,coinselection,coincontrol,scriptpubkeyman,crypter,db,sqlite,walletdb,wallettool,walletutil}.*` + `src/wallet/rpc/*` + `src/wallet/test/` |
| RPC | `src/rpc/{server,request,register,client,blockchain,rawtransaction,mempool,mining,net,node,output_script,util,fees}.*` + `src/httprpc.*`, `src/httpserver.*`, `src/rest.*` |
| P2P | `src/net.{cpp,h}`, `net_processing.*`, `netaddress.*`, `netbase.*`, `netgroup.*`, `net_permissions.*`, `net_types.*` + `src/addrman.*`, `banman.*`, `protocol.*`, `blockencodings.*`, `headerssync.*`, `bip324.*` |
| GUI | `src/qt/*.cpp` (~60: `bitcoingui.cpp`, `bitcoin.cpp`, `walletmodel.cpp`, …) + `src/qt/forms/*.ui`, `locale/*.ts` |
| Krypto | `src/crypto/{sha256,sha512,sha3,ripemd160,aes,chacha20*,poly1305,hkdf_*,hmac_*,muhash,siphash}.*` + `src/secp256k1/` (Subtree) + `src/hash.*`, `pubkey.*`, `key.*`, `musig.*` |

Quelle: Read/Glob-Verifikation am 2026-09-28, HEAD 05bc2f53ce.
