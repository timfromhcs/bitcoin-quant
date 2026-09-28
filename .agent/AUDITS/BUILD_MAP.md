# BUILD_MAP — Baseline (2026-09-28)

- Root: `CMakeLists.txt` (721 Zeilen), `project BitcoinCore 32.99.0`, `CXX20`, `cmake_minimum_required 3.22`
- `CLIENT_NAME "Bitcoin Core"` (`CMakeLists.txt:29`), `CLIENT_VERSION 32.99.0`, `COPYRIGHT_YEAR 2026`
- Optionen: `BUILD_BITCOIN_BIN/DAEMON/GUI/CLI/TESTS/TX/UTIL/GUI_TESTS/UTIL_CHAINSTATE/KERNEL_LIB/KERNEL_TEST/WALLET_TOOL`, `ENABLE_WALLET/EXTERNAL_SIGNER/IPC`, `WITH_ZMQ/QRENCODE/DBUS/USDT/EMBEDDED_ASMAP`, `BUILD_BENCH/FUZZ_BINARY/FOR_FUZZING`, Sanitizer-/PIE-/Fortify-Handling
- Presets (`CMakePresets.json`, 5): `vs2026` (VS 18 2026, x64-windows, GUI+ZMQ), `vs2026-static`, `libfuzzer` (clang, ASan+UBSan+fuzzer), `libfuzzer-nosan`, `dev-mode` (alles ON)
- `vcpkg.json`: baseline `9e593bb1` (2026-07-29), deps `boost-multi-index`, default-features `qt tests wallet zeromq`
- `depends/`: klassisches Cross-Build-System (hosts: darwin/linux/mingw32/freebsd/netbsd/openbsd)
- CI: `ci/test/` (33 Setup-Envs), `.github/workflows/ci.yml` (einziger Workflow)

## Lokale Toolchain (gemessen)

- OS: Microsoft Windows 11 Pro 10.0.26200, Arch AMD64
- cmake 4.4.0, git 2.54.0.windows.1, Python 3.14.6
- Visual Studio: nur `C:\Program Files\Microsoft Visual Studio\2022` vorhanden — Preset verlangt **VS 18 2026** → BUILD VORAUSSICHTLICH BLOCKIERT (wird in Baseline-Build-Probe belegt)
- `VCPKG_ROOT`: leer → vcpkg-Toolchain-File nicht auflösbar → BUILD VORAUSSICHTLICH BLOCKIERT
- `g++`/`cl`/`msbuild` nicht im PATH

Status: BUILD = NOT RUN (Konfigurations-Probe steht aus, siehe BUILD_RESULTS/).
