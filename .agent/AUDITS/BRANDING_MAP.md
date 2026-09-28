# BRANDING_MAP — Baseline (2026-09-28)

- `CLIENT_NAME "Bitcoin Core"` (`CMakeLists.txt:29`), Version 32.99.0, Homepage `https://bitcoincore.org`
- `UA_NAME "Satoshi"` (`src/clientversion.cpp:22`)
- DataDir: Win `%APPDATA%\Bitcoin` (legacy) / `%LOCALAPPDATA%\Bitcoin`, macOS `~/Library/Application Support/Bitcoin`, Unix `~/.bitcoin` (`src/common/args.cpp:859-889`) — KEIN Rebrand
- Qt: `QAPP_ORG_NAME "Bitcoin"`, Domain `bitcoin.org`, `QAPP_APP_NAME_DEFAULT "Bitcoin-Qt"` (+ -testnet/-testnet4/-signet/-regtest) (`src/qt/guiconstants.h:49-55`); Titel-Logik `src/qt/bitcoingui.cpp:1546-1561`
- Binaries: `bitcoind`, `bitcoin-cli`, `bitcoin-tx`, `bitcoin-wallet`, `bitcoin-qt`, `test_bitcoin(-qt)` — unverändert
- Installer: `share/setup.nsi.in` (Name `@CLIENT_NAME@`, `PROGRAMFILES64\Bitcoin`, `bitcoincore.org`, Copyright "The Bitcoin Core developers")
- Qt-Strings: `rg -i bitcoin` in `src/qt/`: ~1198 Zeilen (Top: `bitcoinstrings.cpp:273`, `bitcoingui.cpp:125`)
- Upstream-Legal (KEEP per §19): Copyright-Hinweise, Lizenzen, `secp256k1`-Subtree — kein Blind-Replace.

Rebrand-Pflicht (§103): Icons, Splash, Fenstertitel, Installer, Desktop-Files, Binary-Namen, CLI-Help, Versions-Output, README/Doku.
