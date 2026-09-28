# GUI-Build — Baseline (2026-09-28)

- Re-Configure `build-full` mit `-DBUILD_GUI=ON`: PASS (43.7 s, Qt-Deps aus Manifest-Prefix wiederverwendet)
- Target `bitcoin-qt`: BUILD PASS, `bin/Debug/bitcoin-qt.exe` 45.190.144 Bytes
- Runtime-QA: NOT RUN — `--version` hängt headless (mit und ohne `QT_QPA_PLATFORM=offscreen`,
  je 90–120 s Timeout, Prozess danach gekillt). Wahrscheinlich Debug-Plugin-/Display-Setup;
  kein Code-Fehler belegt. Echte GUI-QA (§123–§125) braucht Display oder Release-Build + Screenshots.
- test_bitcoin-qt-Ziel: NOT BUILT (BUILD_GUI_TESTS default ON bei GUI+Tests? — prüfen im Folgelauf).
