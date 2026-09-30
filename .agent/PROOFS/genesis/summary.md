# PROOF — Genesis Freeze Protocol v1 (§31–§32, 2026-09-28)

## Frozen values (protocol v1, FINAL)
- chain_id QBTC-1, version 1, nBits 0x1d00ffff, timestamp 1758931200 (roll 0)
- nonce 887863234, reward 50 QBTC
- message: "QuantBTC genesis 2026-09-26 - independent chain, no trusted history"
- NUMS pubkey (compressed): 021e4dcc912ccf4638b96d21db73cbf5c992482ccc6f9114140366780801ef6d00
  (label "QuantBTC-genesis-pubkey-v1", counter 0, verifiable by anyone)
- txid/merkle: bab4d3bab87e3ca9493b99d64f7a4db66a9b064ec7128225da032e9bdef2f9c1
- genesis: 000000002f24a967129873ad204d29f947f0452710c72c9aacf45fcf2d8f2881

## §31 checklist
1. Generate twice independently: PASS — run1/run2 manifests IDENTICAL
   (excl. source_commit metadata); logs in backup 05-genesis/
2. Build from clean checkout: PASS 2026-09-30 — worktree of pushed
   9e3240c764 + full configure + full build (bitcoind/test_bitcoin/qt) +
   full suite 877/878, 0 failures, 27.040.867 assertions (log: backup 09-tests/)
3. Genesis test suite: PASS — quantbtc_tests::genesis_block (C++) + 12/12 Python
4. Bitcoin genesis rejected: PASS — C++ cross-network set + Python vectors
5. External backup: PASS — E:/btc quant/quantbtc-backup/05-genesis/ (manifest,
   spec, block/tx hex, both run logs)
6. Manifest: contrib/quantbtc/genesis/generated/genesis-manifest.json (in-repo,
   will be committed)

## Self-healing history (honest record)
- F-GEN-01: first mining failed (empty nonce space, P≈37%) — added bounded
  timestamp-roll (MAX_ROLL 3600s) instead of weakening difficulty
- F-GEN-02: Windows spawn without __main__ guard in diagnostics (process-only)
- F-GEN-03 (CONSENSUS DIVERGENCE, caught by C++ assert): generator used 0x41
  push for 33-byte compressed key; consensus builds 0x21. Fixed generator to
  `bytes([len(pubkey)])`. Python now reproduces C++ txid bab4d3ba EXACTLY.
  Lesson: never trust un-cross-checked serialization constants.
- Mining performance: single-process ~600K H/s → chunk-ledger 16-worker
  ~3.7 MH/s, resumable, scheduling-independent global minimum (~19 min/space)

## Live verification
- bitcoind mainnet: bestblockhash == frozen genesis, chainwork 0x100010001
- C++ assert pins hash + merkle; any drift aborts at startup (fail-closed)
