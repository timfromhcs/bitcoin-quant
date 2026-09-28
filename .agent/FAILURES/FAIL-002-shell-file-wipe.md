# FAIL-002 — Drei Framework-Dateien via Shell-Einzeiler geleert (GEHEILT, kein Datenverlust)

## Symptom
`python -c "io.open(f,'w',...).write(io.open(f).read().replace(...))"` leerte
`address.py`, `wallet.py`, `test_framework.py` (0 Bytes) statt umzubenennen.

## Klassifikation
TOOLCHAIN/PROZESS-Fehler (kein Produkt-Bug): Python wertet `io.open(f,'w')`
(trunkiert) vor `.read()` aus.

## Reparatur
- `git checkout -- <dateien>` (Baseline aus Bundle/HEAD intakt)
- Alle Redos ausschließlich via Edit-Tool + deterministischen Regen-Skripten
- Verifikation: Byte-Größen + `git diff --stat` minimal (26+/26- über 4 Dateien)

## Regel (D-011)
Datei-Edits NUR via Edit/Write-Tools. Shell: read-only, Build, git, Python-Berechnung
ohne Datei-Schreibseite in Einzeilern.
