# QuantBTC Genesis Generator (§30)

Deterministischer Generator für den QuantBTC-Genesisblock (DRAFT-Status, nichts eingefroren).

## Dateien

- `genesis-spec.json` — einzige Eingabe (DRAFT-Werte, `status: DRAFT`)
- `generate_genesis.py` — baut Coinbase-Tx (Spiegel von `CreateGenesisBlock` in
  `src/kernel/chainparams.cpp`), Merkle-Root, minet Nonce per begrenzter Suche,
  schreibt `generated/` + `vectors/genesis-vectors.json`
- `test_genesis_pow.py` — 11 Tests (stdlib-unittest): Bitcoin-Genesis als
  Known-Answer-Vektor, Compact-Edge-Cases, Determinismus (2x-Generierung),
  Bitcoin-Genesis-Reject
- `../ref/pow.py` — Konsens-parameterfreie SHA256d/Compact/Header-Referenz (§114)
- `../ref/difficulty_sim.py` — Difficulty-Vergleichssimulation (§39–§42)

## Verwendung

```bash
python contrib/quantbtc/genesis/generate_genesis.py
python contrib/quantbtc/genesis/test_genesis_pow.py
python contrib/quantbtc/ref/difficulty_sim.py
```

## Hinweise

- DRAFT-Spec nutzt regtest-klasse `nbits` (sofort minbar) und wiederverwendet den
  bekannten (als unspendbar geltenden) Genesis-Pubkey — vor Freeze durch
  Schlüsselzeremonie + Finalwerte ersetzen (§31/§32).
- Freeze-Protokoll §31 (Doppel-Generierung, Clean-Build, Testsuite, externes
  Backup, Manifest) vor Genesis-Freeze Pflicht — aktuell NICHT erfüllt.
