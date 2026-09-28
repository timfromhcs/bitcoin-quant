# GEMINI.md — QuantBTC Zero-to-Hero Autonomous Engineering Master Plan

> **READ THIS FILE FIRST. THEN START.**
>
> This file is the master operating contract for an AI coding agent working inside the local clone of:
>
> `timfromhcs/bitcoin-quant`
>
> The goal is to transform the current Bitcoin-Core-based repository into a genuinely independent **QuantBTC** blockchain project with:
>
> - a new chain and new genesis block
> - independent network identity
> - SHA256d Proof of Work retained as the mining primitive
> - explicit and honest post-quantum transaction security
> - adaptive difficulty and low-hashrate resilience
> - optional merge-mining research path
> - full / pruned / archive / storage node modes
> - lossless compressed state packs
> - native digital asset / NFT functionality
> - decentralized content storage and proof-of-storage accounting
> - bounded automatic storage renewal
> - extended RPC/API surface
> - cross-chain verification adapters
> - modern QuantBTC Core GUI and UX
> - real tests, real builds, real artifacts, real recovery
> - deterministic, reproducible, evidence-driven development
> - self-healing development loops
> - honest README, security documentation, build instructions, and release process.
>
> **The agent must work from evidence. Never pretend. Never fabricate. Never skip a failing critical check. Never hide uncertainty.**

---

# 0. MISSION

Build QuantBTC from the current repository state to a release-grade, independently verifiable blockchain implementation.

The engineering objective is not to make the repository merely compile.

The engineering objective is to reach a state where an independent developer can:

1. inspect the source,
2. reproduce the chain identity,
3. reproduce the genesis block,
4. build the binaries,
5. run a local QuantBTC network,
6. create and restore a wallet,
7. create and transfer PQ-authorized transactions,
8. run full/pruned/storage nodes,
9. create and store digital assets,
10. verify storage proofs,
11. use the RPC API,
12. reproduce critical tests,
13. reproduce release hashes where supported,
14. understand every major security assumption,
15. and verify that QuantBTC is not silently using Bitcoin Mainnet data or identity.

---

# 1. NON-NEGOTIABLE PRINCIPLES

## 1.1 No mocks for real claims

Mocks are allowed only when a test is explicitly a unit test for a component boundary.

Mocks are NOT allowed for:

- mainnet readiness claims
- consensus claims
- genesis verification
- PoW verification
- PQ signature verification
- wallet restore
- storage proofs
- NFT ownership
- state-pack integrity
- final binary validation
- release validation
- testnet operation
- interoperability claims.

Never generate fake:

- hashes
- block heights
- transaction IDs
- storage proofs
- reward figures
- benchmark results
- test results
- CI results
- security audit results
- network statistics
- mainnet data.

If data has not been measured, say:

`NOT MEASURED`

If a test has not run, say:

`NOT RUN`

If a source has not been verified, say:

`NOT VERIFIED`

---

# 2. HONEST SECURITY LANGUAGE

Never claim:

- "100% quantum proof"
- "zero quantum advantage"
- "unbreakable"
- "51% attack impossible"
- "future-proof"
- "mathematically impossible to attack"
- "guaranteed permanent data availability"

while the protocol still uses SHA256d Proof of Work or externally stored content.

QuantBTC must explicitly distinguish:

## Transaction authorization

Post-quantum signature security.

## Proof of Work

SHA256d remains the mining primitive.

Generic quantum search effects are part of the stated security model.

## Majority-PoW attacks

Adaptive difficulty, network resilience and optional merge-mining can improve resilience, but do not make majority control mathematically impossible.

## Storage

On-chain commitments can prove identity/integrity of content, but cannot guarantee that every historical off-chain copy is physically destroyed.

Use precise language.

---

# 3. SOURCE OF TRUTH HIERARCHY

When uncertain, research in this order.

## Level 1 — local repository

Inspect:

- actual source
- local tests
- local build files
- local documentation
- current git state
- current generated artifacts.

Never assume the repository matches upstream documentation.

## Level 2 — upstream official source

Use official Bitcoin Core source, documentation, BIPs and release notes.

## Level 3 — primary standards

For cryptography and security:

- NIST
- IETF
- ISO/IEC where relevant
- formal standards
- official algorithm specifications
- official protocol specifications.

## Level 4 — academic / research sources

For:

- quantum search models
- cryptographic security
- proof-of-storage
- difficulty algorithms
- consensus research.

Prefer peer-reviewed papers and original authors.

## Level 5 — implementation documentation / issues

Use implementation repositories and issue trackers to understand:

- portability
- bugs
- compatibility
- deployment caveats.

Treat issue comments as evidence about implementation behavior, not as formal security proofs.

---

# 4. WEB RESEARCH REQUIREMENT

When an architectural or security question cannot be answered from the repository and stable local documentation:

1. Search multiple independent sources.
2. Prefer primary sources.
3. Search at least two relevant sources for security-critical claims.
4. Search one official implementation source and one independent/primary research or standards source where possible.
5. Record the conclusion and source URLs in the engineering journal.
6. Do not blindly copy claims from search results.
7. Re-check dates for current software versions.
8. If search results disagree, record the disagreement and resolve it from stronger evidence.
9. If uncertainty remains, document it rather than inventing an answer.

For quantum cryptography specifically:

- consult current NIST standards,
- consult the relevant algorithm specification,
- consult the chosen implementation's documentation,
- inspect current portability/build status,
- inspect known issues before selecting a production dependency.

For Bitcoin/Bitcoin-Core behavior:

- inspect local source first,
- then current upstream source,
- then official BIPs/documentation.

---

# 5. TOOL-USAGE POLICY

Use the best available tool for each task.

Potential tools include:

- local shell / PowerShell
- git
- cmake
- ctest
- Python
- static analyzers
- compilers
- sanitizers
- fuzzing
- GitHub inspection
- web search
- image/screenshot-based UI QA
- GUI automation if available
- archive/checksum utilities
- dependency scanners
- secret scanners
- JSON/schema validators.

Do not claim a tool was used if it was not used.

If a tool is unavailable:

1. state the limitation,
2. find a safe alternative,
3. keep the release gate blocked if the unavailable tool is required to make the corresponding claim.

---

# 6. SAFE FILESYSTEM RULES

Before editing anything:

1. determine the exact repository root,
2. determine the exact current working tree,
3. determine the active branch,
4. determine the current HEAD,
5. verify git status,
6. create an external backup.

Do not assume the repository is empty, clean or current.

Never delete files simply because they look unused.

Never overwrite unknown user data.

Never modify files outside the repository unless explicitly required for:
- external backup,
- build artifacts,
- test fixtures,
- temporary working directories,
- release artifacts.

---

# 7. ABSOLUTE FIRST ACTION: BACKUP

Before consensus modifications, create an external backup outside the repository.

Example concept:

```text
../quantbtc-backup/
```

Recommended structure:

```text
quantbtc-backup/
├── 00-baseline/
├── 01-git/
├── 02-source/
├── 03-build/
├── 04-chain/
├── 05-genesis/
├── 06-crypto/
├── 07-storage/
├── 08-nft/
├── 09-tests/
├── 10-security/
├── 11-release/
└── 12-agent-journal/
```

Required baseline data:

- git HEAD
- branch
- upstream reference if available
- working-tree status
- source archive
- git bundle
- hashes of important artifacts
- toolchain information
- build configuration
- baseline test results.

Example commands:

```bash
git status
```

```bash
git rev-parse HEAD
```

```bash
git branch --show-current
```

```bash
git log -1 --oneline --decorate
```

```bash
git bundle create ../quantbtc-backup/01-git/bitcoin-quant-baseline.bundle --all
```

If shell environment differs, adapt safely and document the exact command used.

---

# 8. BACKUP VERIFICATION

A backup is not complete until it can be read.

After creation:

1. verify the backup files exist,
2. verify the git bundle can be listed,
3. calculate hashes where appropriate,
4. compare the recorded HEAD with the original HEAD,
5. record backup verification status.

Example:

```bash
git bundle verify ../quantbtc-backup/01-git/bitcoin-quant-baseline.bundle
```

Do not continue to destructive/consensus-critical work if the baseline cannot be recovered.

---

# 9. BASELINE CHECKPOINT

Create:

```text
.agent/checkpoints/000-baseline/
```

Record:

```text
HEAD
branch
status
build command
build result
unit test result
functional test result
GUI baseline result
toolchain
OS
architecture
```

The checkpoint is immutable.

---

# 10. AGENT STATE FILES

Maintain:

```text
.agent/
├── STATE.md
├── PLAN.md
├── JOURNAL.md
├── DECISIONS.md
├── RISKS.md
├── BLOCKERS.md
├── CHANGES.md
├── TODO.md
├── CURRENT_PHASE.md
├── PROOFS/
├── CHECKPOINTS/
├── TEST_RESULTS/
├── BUILD_RESULTS/
├── AUDITS/
├── RESEARCH/
└── FAILURES/
```

These are development artifacts.

Never put secrets into them.

---

# 11. AGENT STATE MACHINE

The agent follows this state machine:

```text
DISCOVER
  ↓
AUDIT
  ↓
SPECIFY
  ↓
PLAN
  ↓
IMPLEMENT
  ↓
FORMAT / LINT
  ↓
BUILD
  ↓
UNIT TEST
  ↓
INTEGRATION TEST
  ↓
FUNCTIONAL TEST
  ↓
SECURITY TEST
  ↓
FUZZ / SANITIZER
  ↓
VISUAL QA
  ↓
SOURCE AUDIT
  ↓
PROOF GENERATION
  ↓
CHECKPOINT
  ↓
COMMIT
  ↓
PUSH
  ↓
REMOTE CI
  ↓
VERIFY REMOTE RESULT
  ↓
NEXT TASK
```

If any mandatory stage fails:

```text
FAIL
  ↓
CLASSIFY
  ↓
REPRODUCE
  ↓
MINIMIZE
  ↓
PATCH
  ↓
RETEST
```

Never jump over a mandatory stage just to move forward.

---

# 12. DETERMINISTIC LOOP RULE

Every automated loop must have deterministic stop conditions.

Never create an infinite:

```text
while true
```

development process without a bounded failure/exit policy.

Every loop must have:

- explicit success condition,
- explicit failure condition,
- retry count or time budget,
- state checkpoint,
- diagnostic output,
- escalation state.

Recommended pattern:

```text
ATTEMPT
→ OBSERVE
→ CLASSIFY
→ REPAIR
→ RETEST
```

If the same root cause persists after the configured repair budget:

```text
BLOCKED
```

Then:

1. preserve evidence,
2. stop that subtask,
3. document the blocker,
4. continue only with independent tasks that do not depend on the blocker.

For consensus-critical blockers:

```text
STOP PHASE
```

---

# 13. ROOT-CAUSE-FIRST DEBUGGING

Never fix the last error in a long compiler output if an earlier error caused it.

Always:

1. locate the first meaningful failure,
2. inspect the relevant code,
3. reproduce locally,
4. determine the root cause,
5. patch the smallest coherent unit,
6. rerun the failing test,
7. rerun adjacent tests.

Do not randomly modify dozens of files.

---

# 14. FAILURE CLASSIFICATION

Every failure must be classified as one of:

```text
SOURCE BUG
TEST BUG
SPECIFICATION BUG
ENVIRONMENT BUG
DEPENDENCY BUG
TOOLCHAIN BUG
RACE / FLAKY
DATA CORRUPTION
CONSENSUS BUG
SECURITY BUG
DOCUMENTATION BUG
UI BUG
PACKAGING BUG
UNKNOWN
```

`UNKNOWN` must not be silently converted into success.

---

# 15. CONSENSUS-CRITICAL FAILURE POLICY

Treat these as maximum severity:

- genesis calculation
- block hash
- PoW target validation
- difficulty
- timestamp validation
- transaction validity
- signature validation
- PQ signature validation
- UTXO accounting
- monetary issuance
- NFT ownership state
- storage proof validity
- storage reward accounting
- state root / state-pack verification.

On failure:

```text
STOP FEATURE PROGRESS
```

Preserve:

- failing input
- exact block
- exact transaction
- exact configuration
- exact binary/version
- exact expected result
- exact actual result
- relevant logs
- minimal reproducer.

Add a regression test before closing the bug.

---

# 16. NO SILENT TEST SKIPS

Do not:

- disable a failing test,
- mark a test expected-fail without a documented reason,
- reduce a test's scope just to make CI green,
- hide output,
- swallow exceptions,
- return fake success codes.

A skipped test must state:

- why it is skipped,
- what capability is unavailable,
- whether it blocks release,
- how to reproduce it when the environment is fixed.

---

# 17. TEST PASS DEFINITION

A test is "PASS" only if:

1. it actually executed,
2. it reached the intended assertion,
3. the result matched the expected result,
4. logs/artifacts exist where appropriate,
5. no hidden skip occurred.

---

# 18. REPOSITORY FORENSIC AUDIT

Before feature implementation, map:

```text
src/consensus
src/kernel
src/node
src/rpc
src/wallet
src/qt
src/crypto
src/test
test/functional
test/fuzz
share
contrib
doc
.github
```

Generate:

```text
.agent/AUDITS/ARCHITECTURE_MAP.md
.agent/AUDITS/FILE_OWNERSHIP_MAP.md
.agent/AUDITS/BUILD_MAP.md
.agent/AUDITS/TEST_MAP.md
.agent/AUDITS/BRANDING_MAP.md
.agent/AUDITS/NETWORK_IDENTITY_MAP.md
```

---

# 19. BITCOIN IDENTITY SCAN

Search the repository for:

```text
Bitcoin
bitcoin
BITCOIN
BTC
bitcoind
bitcoin-qt
bitcoin-cli
bc
8333
18333
18444
f9beb4d9
Bitcoin DNS seeds
Bitcoin genesis
Bitcoin chainwork
Bitcoin address prefixes
Bitcoin Bech32
Bitcoin silent payment HRP
```

Also inspect:

- hardcoded hashes
- genesis constants
- seeds
- default data paths
- application names
- installer metadata
- product strings
- icons
- desktop entries
- translation strings.

Classify every hit as:

```text
CHANGE
KEEP
LEGAL
UPSTREAM REFERENCE
TECHNICAL ALGORITHM NAME
REVIEW
```

Do not perform a blind global replacement.

---

# 20. UPSTREAM BASELINE COMPARISON

Determine:

- current fork HEAD
- upstream HEAD
- divergence where meaningful
- any local modifications
- any existing Quant/quantum changes already present.

Do not assume the repository is exactly upstream just because it is a fork.

Use git comparisons and source inspection.

Record:

```text
.agent/AUDITS/UPSTREAM_DIFF.md
```

---

# 21. BUILD BASELINE

Run a clean build before substantial modifications.

Capture:

- compiler
- compiler version
- CMake version
- OS
- architecture
- build flags
- optional components
- enabled wallet
- GUI
- test targets.

Do not claim baseline green if only one component built.

---

# 22. MASTER SPECIFICATION

Create:

```text
docs/protocol/quantbtc-spec-v1.md
```

This is the protocol source of truth.

It must define:

- chain ID
- network identifiers
- genesis
- block header
- PoW
- target spacing
- difficulty algorithm
- timestamp rules
- subsidy
- fees
- supply
- wallet output formats
- PQ signature formats
- asset/NFT consensus
- storage proof format
- state commitment
- state-pack format
- network protocol changes
- activation strategy.

---

# 23. DECISION LOG

Every important design decision gets an entry in:

```text
docs/development/DECISIONS.md
```

Each entry contains:

```text
ID
Date
Decision
Problem
Alternatives
Chosen Design
Reasoning
Trade-offs
Security Impact
Tests Required
Evidence
```

Do not silently replace architecture.

---

# 24. PHASE ORDER

Work in this order:

```text
0  Backup
1  Audit
2  Baseline Build
3  Protocol Specification
4  Chain Identity
5  Genesis
6  Consensus Isolation
7  SHA256d PoW
8  Difficulty
9  PQC
10 Wallet
11 Storage
12 State Packs
13 NFT
14 Storage Economy
15 RPC/API
16 P2P
17 Qt UX
18 Cross-chain verification
19 Test system
20 Security hardening
21 Testnet
22 Chaos / long-run testing
23 Release candidate
24 Genesis freeze
25 Reproducible release
26 Mainnet
```

Do not implement NFT economics before the core chain can reliably mine and validate blocks.

---

# 25. CHAIN IDENTITY

Create an explicit QuantBTC identity.

Required:

```text
QuantBTC chain ID
mainnet
testnet
regtest
development network
```

No Bitcoin identity may remain active for the new chain.

---

# 26. NETWORK MAGIC

Create unique QuantBTC network magic bytes.

Verify collision against known networks where practical.

Add tests:

```text
test_quantbtc_magic
test_bitcoin_magic_rejected
test_cross_network_magic_rejected
```

---

# 27. PORTS

Create QuantBTC-specific network ports.

Add collision and configuration tests.

Never rely on Bitcoin Mainnet defaults.

---

# 28. DATA DIRECTORY ISOLATION

Use a QuantBTC-specific data directory.

Example conceptual paths:

```text
Windows:
%APPDATA%\QuantBTC\

Linux:
~/.quantbtc/

macOS:
~/Library/Application Support/QuantBTC/
```

The implementation must use its platform-appropriate conventions.

Tests:

```text
Bitcoin datadir → QuantBTC → reject
QuantBTC datadir → QuantBTC → accept
```

---

# 29. ADDRESS FORMATS

Create independent:

- Base58 prefixes
- Bech32 HRP
- extended key prefixes
- silent-payment prefix where supported.

Tests:

```text
Bitcoin address cannot silently become QuantBTC address
QuantBTC address cannot silently become Bitcoin Mainnet address
```

---

# 30. GENESIS DESIGN

Create a reproducible generator.

Recommended structure:

```text
contrib/quantbtc/genesis/
├── generate_genesis.py
├── genesis-spec.json
├── README.md
├── vectors/
└── generated/
```

The generator must deterministically produce:

- genesis transaction
- genesis block
- merkle root
- block hash
- manifest
- serialization test vector.

---

# 31. GENESIS FREEZE PROTOCOL

Before the final genesis is frozen:

1. generate twice independently,
2. compare outputs,
3. build from clean checkout,
4. execute genesis test suite,
5. verify Bitcoin genesis is rejected,
6. export external backup,
7. record the immutable manifest.

Only then freeze the genesis for that protocol version.

---

# 32. GENESIS MANIFEST

Create:

```text
genesis-manifest.json
```

with:

```text
chain_id
protocol_version
timestamp
version
nBits
nonce
reward
transaction hash
merkle root
genesis hash
generation tool version
source commit
```

Never use placeholder hashes in the final manifest.

---

# 33. CONSENSUS ISOLATION

Remove Bitcoin Mainnet-specific:

- minimum chainwork
- defaultAssumeValid
- AssumeUTXO snapshots
- chain transaction statistics
- Genesis
- activation data that does not apply to QuantBTC
- Bitcoin-specific script exceptions.

Use QuantBTC-specific values or empty values when appropriate.

---

# 34. ASSUMEVALID RULE

For a new chain:

Do not copy Bitcoin Mainnet's `defaultAssumeValid`.

A security optimization must never become an accidental hidden trust anchor.

---

# 35. ASSUMEUTXO RULE

Never copy Bitcoin snapshots.

QuantBTC snapshots may only be introduced after:

- actual QuantBTC chain history exists,
- snapshot state is independently verified,
- snapshot format is versioned,
- snapshot fallback to full validation remains possible.

---

# 36. SHA256D POW

Keep SHA256d as the mining primitive.

The consensus path must be equivalent in principle to:

```text
SHA256(SHA256(block_header))
```

then compare against the compact target.

Do not alter SHA256 internals merely for branding.

---

# 37. POW PROOF SUITE

Implement known vectors for:

- header serialization
- SHA256
- SHA256d
- compact target
- target derivation
- target bounds
- valid PoW
- invalid PoW
- overflow/negative target
- difficulty transition.

Generate machine-readable proof output where useful.

---

# 38. QUANTUM HASH SECURITY CLAIM

The project documentation must state that:

- SHA256d remains PoW,
- no zero-quantum-advantage claim is made,
- generic quantum search assumptions remain part of the security model,
- PQ security is primarily applied to transaction authorization.

Do not describe SHA256d as mathematically immune to quantum speedups.

---

# 39. DIFFICULTY DESIGN

Investigate candidate algorithms with simulations before consensus freeze.

Candidates may include:

- ASERT
- LWMA
- bounded EMA/hybrid
- other well-defined algorithms with strong evidence.

Do not invent a novel controller without a formal model and aggressive attack testing.

---

# 40. DIFFICULTY PROPERTIES

The controller must be tested for:

- deterministic results
- bounded adjustment
- overflow safety
- timestamp manipulation
- timewarp
- oscillation
- miner entry
- miner exit
- long offline periods
- sudden hashrate spikes
- sudden hashrate collapses.

---

# 41. LOW-HASHRATE RESILIENCE

Simulate:

```text
1 miner
2 miners
5 miners
10 miners
100 miner-equivalents
```

and major hashrate loss.

The system must not silently degrade into an unsafe infinite-easy mode.

---

# 42. BLOCK SPACING

Evaluate candidate spacings using real simulations.

Measure:

- propagation latency
- orphan/stale rate
- validation CPU
- bandwidth
- reorg behavior
- wallet UX
- storage growth.

Freeze the result only after evidence exists.

---

# 43. MERGE MINING

Because SHA256d remains:

Investigate optional merge mining.

Separate:

```text
mining support
consensus validation
pool integration
security claims
```

Do not claim merge mining eliminates majority attacks.

Treat it as one layer of security/economic resilience.

---

# 44. 51% MODEL

Document:

- what a majority attacker can do,
- what a minority attacker can do,
- what adaptive difficulty changes,
- what merge mining changes,
- what the node detects,
- what remains possible.

Never state:

`51% attack impossible`.

---

# 45. REORG OBSERVABILITY

Add node/RPC diagnostics for:

- reorg depth
- old tip
- new tip
- affected transactions
- affected asset state
- affected storage state.

GUI should present reorgs clearly without implying that every reorg is automatically malicious.

---

# 46. PQC ARCHITECTURE

Build a clean cryptographic abstraction:

```text
PQCAlgorithm
PQCKeyPair
PQCSignature
PQCVerifier
PQCRegistry
```

The rest of the node should not hardcode one implementation everywhere.

---

# 47. PQC BACKEND LAYER

Use an abstraction:

```text
QuantBTC PQC API
   ↓
Reference backend
Production backend
Test backend
```

The test backend is for controlled unit testing only.

Production claims must use the actual production implementation.

---

# 48. PQC CANDIDATES

Evaluate standardized algorithms such as:

- ML-DSA
- SLH-DSA.

Do not treat a library name as a security proof.

Verify:

- official standard
- parameter set
- implementation source
- current platform support
- known issues
- license
- build integration
- test vectors
- timing/size performance.

---

# 49. PQC PARAMETER SELECTION

Benchmark candidate parameter sets on the actual supported platforms.

Measure:

- public key size
- private key size
- signature size
- sign time
- verify time
- memory
- transaction growth
- wallet responsiveness
- block validation impact.

Do not choose solely by marketing claims.

---

# 50. CRYPTO AGILITY

Every consensus-visible PQ signature format needs:

```text
version
algorithm_id
key encoding
signature encoding
validation rules
```

Future cryptographic migration must be possible without corrupting existing formats.

---

# 51. PQ OUTPUT TYPE

Introduce a clearly versioned post-quantum output path.

Conceptual model:

```text
P2PQ
  version
  algorithm_id
  public-key commitment
```

Define exact serialization in the protocol specification before implementing it.

---

# 52. PQ TEST VECTORS

Include:

- valid signature
- modified message
- modified signature
- wrong public key
- wrong algorithm
- wrong version
- truncated signature
- oversized signature
- malformed encoding.

---

# 53. PQ WALLET

Wallet must support:

- generation
- derivation
- signing
- verification
- backup
- restore
- rescan
- display
- migration where supported.

No private keys in logs.

No private keys in diagnostics.

---

# 54. PQ KEY HANDLING

Use:

- secure memory handling where supported,
- explicit lifetimes,
- wallet locking,
- zeroization facilities where appropriate,
- no accidental serialization into logs or crash reports.

Document platform limitations honestly.

---

# 55. HYBRID SIGNATURE MODE

Optionally research:

```text
PQC + independent PQC
```

or a transitional hybrid policy.

Any hybrid format must be fully specified.

Do not describe hybrid signatures as automatically "twice as secure".

---

# 56. WALLET FIRST-START UX

Design a clear wizard:

```text
Welcome
↓
Create Wallet
Restore Wallet
Run Node Only
Pruned Node
Storage Node
Advanced
```

The default path must remain easy for a normal user.

---

# 57. SIMPLE / ADVANCED UI

Simple:

- balance
- send
- receive
- NFTs
- node status
- storage status.

Advanced:

- peers
- RPC
- indexes
- PQC details
- storage diagnostics
- chain information
- debugging
- logs.

---

# 58. NODE MODES

Support clearly defined profiles:

```text
Full Node
Pruned Node
Archive Node
Storage Node
Hybrid Node
Mining Node
Wallet Node
```

Roles may overlap.

---

# 59. HARDWARE-AWARE SETTINGS

At startup, detect:

- RAM
- CPU
- disk capacity
- available disk
- network constraints.

Then choose safe policy defaults.

Never change consensus dynamically.

---

# 60. RESOURCE LIMITS

Expose safe policy settings for:

- DB cache
- maximum connections
- mempool size
- bandwidth
- verification threads
- storage cache
- NFT cache.

---

# 61. PRUNING

Use pruning as an explicit first-class mode.

GUI must explain:

- what is deleted,
- what remains,
- how much storage is expected,
- that historical data may need to be downloaded again to return to archive/full history.

---

# 62. PRUNING / INDEX COMPATIBILITY

Test every relevant combination:

```text
pruned + wallet
pruned + block filters
pruned + NFT
pruned + storage
pruned + state packs
pruned + RPC
```

If a combination is impossible, fail clearly rather than silently.

---

# 63. STATE PACK DESIGN

Do not compress a live database by simply archiving LevelDB files.

Use:

```text
logical state
→ deterministic serialization
→ chunking
→ compression
→ manifest
→ content hashes
```

---

# 64. STATE PACK FORMAT

Conceptually:

```text
.qsp
```

with:

```text
HEADER
MANIFEST
INDEX
CHUNKS
FOOTER
```

Version the format.

---

# 65. STATE PACK MANIFEST

Include:

```text
chain_id
protocol_version
height
block_hash
state_root
pack_root
chunk_count
compression
compressed_bytes
uncompressed_bytes
generator_version
```

---

# 66. LOSSLESS REQUIREMENT

Before compression:

```text
state_root_before
```

After decompression/reconstruction:

```text
state_root_after
```

Required:

```text
state_root_before == state_root_after
```

Also compare relevant record counts and deterministic serialized representations.

---

# 67. CHUNKING

Use deterministic chunking.

Research a reasonable chunk size based on:

- throughput
- RAM
- corruption recovery
- parallel decompression
- partial transfer
- disk IO.

Benchmark real chainstate data.

---

# 68. MULTIPLE COMPRESSION BACKENDS

Abstract:

```text
CompressionBackend
```

Possible implementations:

```text
none
zstd
lz4
future
```

Compression must not change consensus state.

---

# 69. STATE PACK FALLBACK

State packs are an optimization.

If a state pack:

- fails integrity,
- is unsupported,
- is corrupt,
- mismatches the chain,
- cannot be imported,

the node must be able to fall back to normal synchronization.

State pack failure must not imply blockchain failure.

---

# 70. NFT PROTOCOL

Do not put arbitrary huge media blobs directly into every transaction.

Separate:

```text
ownership
metadata
content commitment
storage contract
```

from:

```text
actual large content
```

---

# 71. NFT CORE RECORD

At minimum, define:

```text
asset_id
collection_id
owner
creator
content_root
metadata_root
created_height
storage_policy
status
```

---

# 72. CONTENT ADDRESSING

Large content:

1. chunk,
2. hash each chunk,
3. build authenticated manifest,
4. derive content root,
5. store through decentralized storage providers.

The chain stores the commitment and ownership logic.

---

# 73. NFT LIFECYCLE

Suggested states:

```text
MINTED
ACTIVE
RENEWAL_SOON
RENEWED
GRACE
EXPIRED
UNPINNED
BURNED
```

Exact semantics belong in the consensus specification.

---

# 74. STORAGE LEASE

The owner chooses:

- lease period,
- replication,
- budget,
- renewal policy.

Use an explicit bounded budget, not unlimited automatic spending.

---

# 75. AUTOMATIC RENEWAL

Every automated renewal must have:

- explicit opt-in,
- max spend per period,
- max spend per asset,
- optional lifetime cap,
- minimum balance reserve,
- visible next renewal,
- cancel control,
- audit history.

---

# 76. FAILED RENEWAL

Recommended flow:

```text
ACTIVE
↓
RENEWAL_SOON
↓
PAYMENT FAILED
↓
GRACE
↓
EXPIRED
↓
UNPINNED
```

Do not immediately destroy user state.

---

# 77. PHYSICAL DELETION CLAIM

Never claim that protocol expiry can physically erase every historical copy of content.

Use terms such as:

- expired,
- unpinned,
- no longer rewarded,
- unavailable,
- burned.

---

# 78. STORAGE PROVIDERS

A storage provider may register:

```text
provider_id
payout_address
proof key
capacity commitment
pricing
protocol version
status
```

---

# 79. PROOF OF STORAGE

A provider must prove actual possession/retrievability of selected content.

A proof system should have:

```text
challenge
→ selected data
→ cryptographic proof
→ deterministic verification
→ reward decision
```

Do not reward an unverified storage claim.

---

# 80. CHALLENGE ENTROPY

Challenges should come from protocol-defined entropy or a clearly specified source that providers cannot manipulate unilaterally.

Avoid provider-controlled randomness.

---

# 81. STORAGE REWARDS

Reward must be deterministic.

Inputs might include:

- verified bytes
- proof success
- service uptime/availability where provable
- replica requirements
- lease state.

Do not create arbitrary admin-controlled payments.

---

# 82. SYBIL RESISTANCE

Multiple identities must not cheaply create unlimited rewards.

Evaluate:

- capacity commitments
- collateral/deposit where appropriate
- cost of storage
- proof obligations
- replication diversity
- anti-replay rules.

---

# 83. STORAGE PROVIDER FAILURE

When a provider misses proofs:

```text
proof failures
→ reward paused/reduced according to protocol
→ degraded provider state
→ migration / repair
→ replacement provider
```

Define exact thresholds in the protocol.

---

# 84. PROVIDER EXIT

Support a controlled exit:

```text
announce exit
→ grace period
→ data migration
→ final state
```

Avoid sudden loss of required replicas.

---

# 85. STORAGE / PRUNING SEPARATION

Do not conflate:

```text
historical blockchain pruning
```

with:

```text
current content storage.
```

They are separate storage classes.

---

# 86. NFT SPAM CONTROL

Define protocol/policy limits for:

- asset size
- metadata size
- transaction size
- minimum fees
- storage deposits
- replication
- provider challenge cost.

Do not allow a single cheap transaction to cause unbounded storage cost.

---

# 87. STORAGE ECONOMICS MODEL

Design:

```text
Owner
  ↓
Storage Escrow
  ↓
Verified Providers
  ↓
Proofs
  ↓
Deterministic Reward Split
```

Reward calculations must have:

- explicit formula,
- rounding rules,
- overflow rules,
- maximum values,
- state transition tests.

---

# 88. MONETARY POLICY

Freeze before mainnet:

- max supply or emission policy
- block reward
- halving/emission schedule
- transaction fees
- NFT fees
- storage fees
- burn semantics if any.

Never improvise monetary behavior during UI development.

---

# 89. RPC/API ARCHITECTURE

Keep new API logic modular.

Suggested areas:

```text
src/rpc/
├── quant.*
├── pq.*
├── nft.*
├── storage.*
└── snapshot.*
```

Do not turn one massive file into an unmaintainable feature dump.

---

# 90. RPC VERSIONING

Version the schemas.

Do not silently break existing clients.

Every breaking API change requires:

- version increment,
- migration documentation,
- tests,
- release notes.

---

# 91. CAPABILITY DISCOVERY

Provide a capability query that reports actual features.

Conceptual fields:

```text
pqc
nft
storage
statepack
merge_mining
protocol_version
```

The GUI must use this instead of guessing.

---

# 92. API SCHEMAS

Generate or maintain machine-readable schemas for:

- RPC responses
- request parameters
- error codes
- event payloads.

Where possible, generate docs from these schemas.

---

# 93. ERROR MODEL

Every error should have:

```text
error code
technical message
user message
suggested action
```

Example classes:

```text
ERR_STORAGE_BUDGET_LOW
ERR_STATEPACK_MISMATCH
ERR_PQC_UNSUPPORTED
ERR_CHAIN_ID_MISMATCH
ERR_REORG_DETECTED
```

---

# 94. FRONTEND / BACKEND CONTRACT

Architecture:

```text
Qt UI
 ↓
ViewModel / Controller
 ↓
QuantBTC Service Layer
 ↓
RPC Client
 ↓
Node
 ↓
Core
```

The GUI must not directly modify consensus state.

---

# 95. TYPED DTOs

Define explicit models such as:

```text
ChainInfo
NodeHealth
WalletInfo
PQCInfo
NFTInfo
StorageInfo
SnapshotInfo
```

Avoid untyped dictionary spaghetti across the UI.

---

# 96. EVENT-DRIVEN UI

Expose events for:

- new block
- sync state
- wallet change
- NFT change
- provider status
- storage proof
- reorg
- diagnostic result.

Avoid excessive polling where an event model is appropriate.

---

# 97. GUI HEALTH CENTER

Provide:

```text
Node
Wallet
PQC
Storage
Sync
Peers
Disk
Database
Security
```

with honest status:

```text
Healthy
Warning
Action Required
Unknown
```

Never display a fake "Secure" green indicator when evidence is unavailable.

---

# 98. GUI DIAGNOSTICS

Add a diagnostics action that actually checks:

- network
- ports
- datadir
- database
- chainstate
- wallet
- PQC backend
- storage
- disk
- RPC.

Export a sanitized report.

---

# 99. ONE-CLICK REPAIR

Only automate safe repairs, for example:

- rebuild an index
- verify chainstate
- redownload missing block
- rebuild derived cache
- retry state-pack import.

Never automatically:

- delete wallets,
- rewrite consensus,
- overwrite user data,
- expose secrets.

---

# 100. EXPLAIN BEFORE DESTRUCTIVE ACTION

Any operation that may:

- delete historical data,
- reindex,
- migrate wallet/db formats,
- change node mode,

must explain:

- what happens,
- what remains,
- expected disk/CPU use,
- whether a backup exists,
- whether rollback is possible.

---

# 101. ACCESSIBILITY

Support where practical:

- keyboard navigation
- focus states
- scalable text
- screen-reader labels
- high contrast
- reduced motion
- status indicators independent of color alone.

---

# 102. LOCALIZATION

Architect for:

- English
- German
- French

Use translation infrastructure instead of hardcoded UI strings.

---

# 103. BRANDING

Replace user-facing Bitcoin branding with QuantBTC branding.

Audit:

```text
icons
splash
window title
installer
desktop files
binaries
strings
README
docs
CLI help
version output
```

Do not remove required upstream legal notices or third-party licenses.

---

# 104. BINARY NAMES

Target names conceptually include:

```text
quantbtcd
quantbtc-cli
quantbtc-tx
quantbtc-qt
```

Only create additional binaries when they have a real function.

---

# 105. INSTALLER

Windows installer must:

- install correct binaries,
- install correct icons,
- use QuantBTC product metadata,
- use QuantBTC paths,
- create proper shortcuts,
- uninstall cleanly,
- preserve user wallet data according to documented uninstall policy.

---

# 106. UPGRADE / MIGRATION

For database or wallet format changes:

```text
detect
→ backup
→ migrate
→ verify
→ activate
```

On failure:

```text
restore
→ report
```

Never "hope" an old database opens correctly.

---

# 107. P2P CAPABILITY NEGOTIATION

Peers should advertise actual capabilities, such as:

```text
protocol
PQC
NFT
storage
statepack
```

All protocol-visible capabilities must be versioned.

---

# 108. P2P RESILIENCE

Test:

- peer churn
- seed failure
- DNS failure
- slow peers
- malicious peers
- network partition
- reconnect
- invalid blocks
- invalid transactions.

---

# 109. CROSS-CHAIN

Implement read/verification functionality before bridges.

Architecture:

```text
IExternalChain
IBlockVerifier
ITransactionVerifier
IMerkleProofVerifier
```

Initial work may cover:

- Bitcoin
- Litecoin
- Dogecoin
- Ethereum,

but only where there is a real need and verifiable protocol support.

---

# 110. NO TRUSTLESS BRIDGE BY ACCIDENT

A parser is not a verifier.

A verifier is not a bridge.

A bridge is not automatically trustless.

Document the exact trust model for every cross-chain component.

---

# 111. TEST ARCHITECTURE

Use layered tests:

```text
L0 syntax/lint
L1 build
L2 unit
L3 integration
L4 functional
L5 P2P
L6 wallet
L7 PQC
L8 storage
L9 NFT
L10 consensus
L11 security
L12 fuzz/sanitizer
L13 GUI/visual
L14 packaging
L15 release
```

---

# 112. UNIT TESTS

Cover:

- serialization
- PoW
- difficulty
- scripts
- PQC
- storage formats
- compression
- NFT state
- reward calculations.

---

# 113. CONSENSUS PROPERTY TESTS

Test properties such as:

```text
same input → same result
```

and:

- no overflow
- no invalid negative values
- bounded difficulty
- deterministic reward calculation
- deterministic NFT state transitions
- deterministic storage proof verification.

---

# 114. DIFFERENTIAL TESTING

Create a second lightweight reference implementation for critical formulas where practical.

Compare:

```text
Core
vs
Reference
```

for:

- difficulty
- serialization
- PQ encoding
- NFT transitions
- storage reward calculation.

---

# 115. FUZZING

Create or extend fuzz targets for:

```text
genesis
block headers
PoW
difficulty
PQC serialization
PQC verification
NFT manifests
storage proofs
state packs
RPC inputs
P2P messages
```

---

# 116. SANITIZERS

Run relevant:

```text
ASAN
UBSAN
TSAN
```

and other supported sanitizers.

Use sanitizer findings to create permanent regression tests.

---

# 117. FAULT INJECTION

Simulate:

- process crash
- disk full
- truncated file
- corrupted chunk
- network loss
- peer loss
- corrupted database
- wallet lock
- interrupted snapshot import.

Verify deterministic recovery.

---

# 118. RECOVERY TEST

A successful recovery test is:

```text
create state
→ interrupt
→ restart
→ verify chain
→ verify state
→ continue operation
```

Not merely:

```text
process restarted
```

---

# 119. MULTI-NODE FUNCTIONAL TESTS

Use real nodes:

```text
node0
node1
node2
node3
...
```

not fake response servers.

Test:

- sync
- transaction propagation
- mining
- reorg
- wallet
- NFT
- storage.

---

# 120. REAL TESTNET

After local regtest:

- deploy a real testnet,
- use real P2P,
- use real blocks,
- use real wallet transactions,
- use real PQ signatures,
- use real NFT operations,
- use real storage proofs.

Do not call a local mock environment "public testnet".

---

# 121. LONG-RUN TESTS

Run long-lived nodes.

Suggested milestones:

```text
24h
72h
7d
```

Measure:

- memory leaks
- disk growth
- peer churn
- block validation
- database health
- storage reliability
- CPU utilization.

---

# 122. CHAOS TESTNET

Vary:

- peers offline
- latency
- packet loss
- CPU pressure
- disk pressure
- storage provider failures
- miner entry/exit.

Record evidence.

---

# 123. GUI VISUAL QA

Visual QA is mandatory for every major UI change.

Use actual built application.

Validate:

- launch
- navigation
- dialogs
- text clipping
- icons
- spacing
- scaling
- dark/light behavior if supported
- responsive states
- disabled/error/loading states
- long values
- error messages.

---

# 124. SCREENSHOT REGRESSION

For stable pages:

```text
.visual-baselines/
.visual-current/
.visual-diffs/
```

Compare major UI screens.

Do not use screenshot comparison alone; combine it with behavioral checks.

---

# 125. END-TO-END GUI TEST

At minimum:

```text
launch
→ create wallet
→ create/receive address
→ send transaction
→ observe confirmation
→ inspect node status
→ inspect PQC
→ open NFT view
→ inspect storage
→ restart
→ confirm state.
```

---

# 126. CODE QA

For changed code:

1. format,
2. compile,
3. lint,
4. static analysis,
5. targeted unit tests,
6. neighboring tests,
7. integration tests.

Never stop at compilation.

---

# 127. CHANGE-SCOPE DISCIPLINE

Prefer:

```text
small coherent feature
```

over:

```text
massive unrelated refactor
```

A consensus patch should not also rename 500 UI strings unless there is a real reason.

---

# 128. COMMIT DISCIPLINE

Good commits are coherent.

Examples:

```text
consensus: add QuantBTC chain identity
```

```text
genesis: add deterministic QuantBTC genesis generator
```

```text
pqc: add versioned P2PQ validation path
```

```text
storage: add verified state-pack format
```

Do not use vague commit messages like:

```text
fix stuff
```

---

# 129. CHECKPOINTING

After every major coherent milestone:

```text
.agent/checkpoints/<number>-<name>/
```

Record:

- commit
- tests
- artifacts
- known risks
- rollback strategy.

Create the external backup before committing/pushing critical milestones.

---

# 130. EXTERNAL ARTIFACT POLICY

Large/generated artifacts stay outside Git unless there is a documented reason.

Examples:

- blockchains
- wallets
- state packs
- testnet snapshots
- large NFT datasets
- generated binary archives.

Use release artifacts or dedicated object storage where needed.

Never accidentally commit user data.

---

# 131. SECRET SCANNING

Before every public push scan for:

- private keys
- seed phrases
- RPC credentials
- passwords
- tokens
- API keys
- TLS private keys
- cloud credentials.

If detected:

```text
STOP PUSH
```

---

# 132. LARGE FILE SCAN

Before every push look for:

- wallet databases
- block databases
- giant state packs
- generated test artifacts
- accidental binaries,
- logs.

---

# 133. GIT DIFF GATE

Before commit:

```bash
git status
```

```bash
git diff --check
```

```bash
git diff
```

Also inspect staged diff.

The agent must understand every modified file.

---

# 134. PRE-PUSH TEST GATE

Before pushing a significant change:

```text
build
→ targeted tests
→ relevant full tests
→ security checks
→ source scan
→ backup
→ commit
→ push
```

---

# 135. PUSH POLICY

Push only when:

- working tree is understood,
- no secrets are present,
- local required tests pass,
- external checkpoint exists,
- commit is coherent,
- documentation is updated,
- no critical blocker exists.

---

# 136. REMOTE CI IS A NEW TEST STAGE

After push:

```text
remote CI
→ inspect actual status
→ inspect failed job
→ inspect logs
→ reproduce locally
```

Do not assume CI passes because local tests passed.

---

# 137. CI FAILURE LOOP

```text
CI FAIL
→ identify first failed job
→ inspect logs
→ reproduce
→ classify
→ fix
→ local verify
→ backup
→ commit
→ push
```

Never repeatedly rerun CI without understanding the cause.

---

# 138. FLAKY TEST HANDLING

If a test appears flaky:

1. repeat it,
2. record frequency,
3. inspect race/timing,
4. reproduce under stress,
5. fix root cause or document environment limitation.

Do not simply delete the test.

---

# 139. DOCUMENTATION AUTOMATION

Whenever protocol behavior changes, check:

```text
README
QUANTBTC.md
protocol docs
API docs
security docs
operator docs
release notes
```

Do not let documentation drift.

---

# 140. README FINAL STANDARD

README must clearly contain:

```text
What QuantBTC is
Current status
Architecture
Security model
Quantum model
Mining
Wallet
Nodes
Pruning
State packs
NFTs
Storage
RPC
Build
Test
Run
Release
Known limitations
License
```

---

# 141. README STATUS MUST BE REAL

Use states such as:

```text
Development
Testnet
Release Candidate
Mainnet
```

Never claim "production ready" while critical release gates remain open.

---

# 142. README BUILD SECTION

Use the actual build system and commands from the repository.

Do not invent commands.

Typical CMake structure may be:

```bash
cmake -B build
```

```bash
cmake --build build
```

```bash
ctest --test-dir build
```

But the agent must verify the exact valid commands from the actual repository before publishing the final instructions.

---

# 143. README QUICKSTART

A clean user should be able to understand:

```text
install
→ start node
→ create/restore wallet
→ sync
→ receive QBTC
```

---

# 144. OPERATOR DOCS

Provide:

```text
full-node.md
pruned-node.md
archive-node.md
storage-node.md
mining.md
backup-and-recovery.md
```

---

# 145. DEVELOPER DOCS

Provide:

```text
build.md
architecture.md
consensus.md
pqc.md
statepack.md
nft.md
storage.md
rpc.md
testing.md
release.md
```

---

# 146. SECURITY DOCUMENTATION

Provide:

```text
quantum-security.md
pow-security.md
wallet-security.md
p2p-security.md
storage-security.md
nft-security.md
threat-model.md
```

---

# 147. SECURITY EVIDENCE

Every important claim should have:

```text
CLAIM
ASSUMPTION
TEST
EVIDENCE
LIMITATION
```

Example:

```text
Claim:
Bitcoin Genesis is rejected by QuantBTC.

Assumption:
Consensus parameters are loaded from QuantBTC chain params.

Test:
cross_genesis_rejection.py

Evidence:
test result artifact
```

---

# 148. QUANTUM EVIDENCE

Generate:

```text
.agent/PROOFS/quantum-security/
```

with:

- standards references
- algorithm selection evidence
- test vectors
- implementation version
- benchmark results
- negative tests
- threat model
- remaining limitations.

Do not invent mathematical proofs that do not exist.

---

# 149. POW EVIDENCE

Generate:

```text
.agent/PROOFS/pow/
```

with:

- SHA256d vectors
- target vectors
- difficulty vectors
- header vectors
- simulation results
- attack-model notes.

---

# 150. GENESIS EVIDENCE

Generate:

```text
.agent/PROOFS/genesis/
```

with:

- spec
- generator version
- raw genesis
- transaction
- merkle root
- block hash
- reproducibility result
- external archive reference.

---

# 151. STATE-PACK EVIDENCE

Generate:

```text
.agent/PROOFS/statepack/
```

with:

- source state hash
- serialized state hash
- compressed size
- decompressed size
- reconstruction hash
- equality result
- benchmark
- corrupted-input tests.

---

# 152. STORAGE EVIDENCE

Generate:

```text
.agent/PROOFS/storage/
```

with:

- provider registration test
- challenge
- proof
- verification
- reward calculation
- failure recovery
- replication tests.

---

# 153. NFT EVIDENCE

Generate:

```text
.agent/PROOFS/nft/
```

with:

- mint
- transfer
- content root
- lease
- renewal
- expiration
- provider failure
- burn
- reorg behavior.

---

# 154. RELEASE EVIDENCE

Generate:

```text
.agent/PROOFS/release/
```

with:

- source commit
- chain manifest
- genesis manifest
- build matrix
- tests
- sanitizer results
- fuzz results
- package hashes
- installer test
- reproducibility report.

---

# 155. SOURCE AUDIT BEFORE RELEASE

Scan for:

```text
Bitcoin identifiers
old Genesis
old network magic
old ports
old addresses
old seeds
old datadir
old binary names
stale docs
stale examples
secrets
```

Every remaining occurrence must be classified.

A remaining upstream copyright notice is not automatically a bug.

---

# 156. CONSENSUS / UI SEPARATION

Do not place consensus decisions in GUI code.

The GUI requests actions.

Core validates.

Example:

```text
UI:
"Renew NFT"

Service:
build renewal request

RPC:
submit

Core:
verify consensus rules

Wallet:
sign if authorized

Network:
broadcast
```

---

# 157. NO BUSINESS LOGIC DUPLICATION

If the same rule exists in:

```text
C++
Python
Qt
```

make one authoritative source and use tests to keep secondary representations synchronized.

---

# 158. UI DATA MUST COME FROM REAL NODE STATE

No hardcoded:

```text
"Storage: Healthy"
```

No fake:

```text
"12 providers online"
```

Every visible state should derive from real node/backend data or be labeled as unavailable.

---

# 159. DEMO MODE

If a demo mode is ever needed, it must be:

- clearly marked,
- separate from production mode,
- never used as evidence for mainnet readiness.

---

# 160. PERFORMANCE TESTS

Benchmark:

- block validation
- transactions/sec
- PQ sign/verify
- state-pack compression/decompression
- NFT processing
- storage proofs
- RPC latency
- GUI responsiveness
- disk usage
- RAM.

Record hardware and software versions.

---

# 161. BENCHMARK HONESTY

Every benchmark result includes:

```text
hardware
OS
compiler
commit
configuration
dataset
run count
mean/median where applicable
```

Do not compare results from incompatible hardware without saying so.

---

# 162. STORAGE COMPRESSION BENCHMARK

Compare:

```text
raw
zstd low
zstd balanced
zstd high
lz4
```

on real representative data.

Measure:

```text
compression ratio
compression time
decompression time
RAM
disk
verification time
```

---

# 163. LOW-RESOURCE MODE

QuantBTC should remain useful on modest devices.

Safe optimizations include:

- lower cache
- fewer peers
- smaller mempool
- delayed optional indexing
- pruning
- state packs
- storage cache limits.

Never disable mandatory validation.

---

# 164. BACKUP / RESTORE UI

Wallet backup should show:

```text
Create Backup
Verify Backup
Restore Backup
```

Do not imply that an unverified copy is a valid backup.

---

# 165. DIAGNOSTIC SANITIZATION

Generated diagnostics must exclude:

- private keys
- seed phrases
- wallet secrets
- RPC passwords
- auth tokens.

---

# 166. DATABASE CORRUPTION MODEL

When corruption is detected:

```text
detect
→ classify
→ backup current state if possible
→ stop unsafe writes
→ offer verified recovery path
```

Do not continue writing through known corruption.

---

# 167. CRASH RECOVERY

Test interruption at:

- block write
- chainstate flush
- snapshot export
- snapshot import
- NFT state update
- storage bookkeeping.

---

# 168. REORG / NFT CONSISTENCY

A reorg must correctly restore:

- UTXO state
- NFT ownership
- storage lease state
- reward accounting
- provider eligibility.

---

# 169. REORG / STORAGE CONSISTENCY

Any storage reward determined by consensus must be reorg-safe.

A reverted block must not leave behind an irreversible local reward state unless the protocol explicitly defines such behavior.

---

# 170. API SECURITY

All new RPCs must validate:

- sizes
- paths
- numeric ranges
- chain identity
- authentication/permissions where needed
- malformed JSON
- replay-sensitive parameters
- resource consumption.

---

# 171. DOS PROTECTION

A malicious request must not cause uncontrolled:

- CPU work
- memory usage
- disk allocation
- network traffic
- PQ signature verification
- content storage.

---

# 172. RESOURCE BUDGETS

Every expensive operation should have a measurable upper bound or policy limit where practical.

Particularly:

- PQ verification
- storage proof checks
- state-pack imports
- NFT processing
- RPC batch sizes.

---

# 173. REGTEST ENHANCEMENTS

Create QuantBTC-specific development controls for:

- easy mining
- deterministic timestamps
- fault injection
- storage-provider simulation
- NFT tests
- PQ tests
- state-pack tests.

Development convenience must remain clearly separate from production consensus.

---

# 174. TESTNET CONFIGURATION

Testnet must use:

- real network parameters,
- real peer connectivity,
- real chain history,
- real cryptographic operations.

Do not accidentally ship regtest settings as testnet.

---

# 175. MAINNET CONFIGURATION

Mainnet values are frozen.

No debug shortcut may accidentally disable:

- PoW validation
- signature checks
- state checks
- network identity.

---

# 176. MAINNET SECURITY FREEZE

Once release candidate begins:

```text
NO NEW PROTOCOL FEATURES
```

Only:

- security fixes
- consensus fixes
- build fixes
- documentation fixes
- release fixes.

---

# 177. RELEASE CANDIDATE

Use:

```text
v1.0.0-rc1
```

and subsequent RCs as needed.

Each RC must be installable on a clean environment.

---

# 178. CLEAN-MACHINE VALIDATION

On a clean system:

1. install,
2. start,
3. create wallet,
4. run node,
5. sync,
6. transact,
7. inspect assets,
8. stop,
9. restart,
10. verify state.

---

# 179. WINDOWS

Release candidate must test:

```text
quantbtcd
quantbtc-cli
quantbtc-qt
```

including GUI and wallet.

---

# 180. LINUX

Test:

- daemon
- CLI
- wallet
- GUI where supported
- service operation.

---

# 181. OPTIONAL MACOS / ARM

Add when supported by the actual build environment.

Do not publish "supported" when only cross-compilation succeeded without runtime validation.

---

# 182. REPRODUCIBLE BUILDS

The release target is:

```text
same source
+
same toolchain
+
same dependencies
+
same build configuration
=
same artifact
```

Where perfect reproduction is not yet achieved, publish the exact verified scope and limitation.

---

# 183. RELEASE MANIFEST

Create:

```text
release-manifest.json
```

with:

- source commit
- protocol version
- genesis hash
- chain ID
- PQC algorithm identifiers
- PoW
- supported platforms
- artifact hashes.

---

# 184. ARTIFACT CHECKSUMS

Generate checksums for:

- daemon
- CLI
- GUI
- installer
- source archive
- relevant release packages.

Use SHA-256 for artifact integrity if chosen; this is separate from the PoW security claim.

---

# 185. RELEASE SIGNING

Keep release-signing credentials outside the repository.

Sign the release manifest and/or tags using an auditable release process.

Never commit private release keys.

---

# 186. PUSH ORDER

Use coherent feature pushes.

Suggested order:

```text
protocol specification
↓
chain identity
↓
genesis
↓
consensus
↓
PoW/difficulty
↓
PQC
↓
wallet
↓
storage
↓
state packs
↓
NFT
↓
storage economy
↓
RPC
↓
P2P
↓
GUI
↓
testnet tooling
↓
release tooling
```

The exact order may change when dependencies require it, but the agent must document the reason.

---

# 187. BEFORE EVERY PUSH

Run, as appropriate:

```bash
git status
```

```bash
git diff --check
```

```bash
git diff
```

Then:

- build,
- targeted tests,
- relevant full tests,
- security scans,
- secret scan,
- source audit,
- external backup/checkpoint.

---

# 188. AFTER EVERY PUSH

Inspect:

- remote branch,
- commit,
- CI,
- failed jobs,
- artifacts,
- generated release/report files.

A push is not "done" until remote verification is complete.

---

# 189. NEVER PUSH A KNOWN BROKEN CONSENSUS CHANGE

If a consensus-critical test fails:

```text
DO NOT PUSH
```

unless the push is intentionally to a clearly marked development branch specifically for debugging and the README/branch status makes that explicit.

---

# 190. BRANCH STRATEGY

Prefer:

```text
main/master
```

for stable work,

and feature branches such as:

```text
feat/chain-identity
feat/genesis
feat/pqc
feat/storage
feat/statepack
feat/nft
feat/rpc
feat/qt
```

Avoid force-push on stable branches.

---

# 191. UPSTREAM UPDATES

Do not blindly merge future upstream Bitcoin changes.

When upstream changes:

1. inspect the change,
2. identify affected QuantBTC subsystems,
3. run source diff,
4. assess consensus impact,
5. re-run tests,
6. document adaptation.

---

# 192. SECURITY PATCHES FROM UPSTREAM

Treat upstream security fixes as high priority.

But adapt them carefully to QuantBTC modifications.

Never copy-paste without checking interaction with custom consensus.

---

# 193. AGENT SELF-HEALING LOOP TEMPLATE

For every feature:

```text
READ TASK
→ READ SPEC
→ INSPECT CODE
→ FIND EXISTING IMPLEMENTATION
→ DESIGN PATCH
→ IMPLEMENT
→ BUILD
→ RUN TARGETED TESTS
→ RUN RELATED TESTS
→ RUN SECURITY TESTS
→ RUN UI/API TESTS
→ REVIEW DIFF
→ GENERATE PROOF
→ CHECKPOINT
→ COMMIT
→ PUSH
→ VERIFY CI
```

On failure:

```text
FAIL
→ SAVE EVIDENCE
→ ROOT-CAUSE CLASSIFY
→ REPRODUCE
→ MINIMIZE
→ PATCH
→ REGRESSION TEST
→ RETEST
```

---

# 194. AGENT RESEARCH LOOP

For unresolved technical questions:

```text
LOCAL SOURCE
→ OFFICIAL UPSTREAM
→ OFFICIAL STANDARD
→ PRIMARY RESEARCH
→ IMPLEMENTATION ISSUES
→ COMPARE
→ DECIDE
→ DOCUMENT
```

At least two sources for security-critical conclusions whenever practical.

---

# 195. AGENT CODE LOOP

For a code change:

```text
SEARCH
→ READ CONTEXT
→ IDENTIFY DEPENDENCIES
→ PATCH MINIMALLY
→ FORMAT
→ COMPILE
→ TEST
→ INSPECT DIFFERENCE
```

Never edit code from a search-result snippet without reading the surrounding implementation.

---

# 196. AGENT UI LOOP

For a UI feature:

```text
MODEL / API
→ BACKEND
→ SERVICE LAYER
→ VIEWMODEL
→ UI
→ UNIT TEST
→ INTEGRATION TEST
→ GUI TEST
→ SCREENSHOT QA
→ ERROR-STATE QA
```

---

# 197. AGENT STORAGE LOOP

For storage:

```text
FORMAT
→ SERIALIZE
→ HASH
→ COMPRESS
→ WRITE
→ READ
→ DECOMPRESS
→ HASH
→ COMPARE
→ CORRUPT
→ DETECT
→ RECOVER
```

---

# 198. AGENT NFT LOOP

```text
CREATE
→ VALIDATE
→ SIGN
→ BROADCAST
→ MINE
→ CONFIRM
→ INDEX
→ STORE
→ PROVE
→ RENEW
→ EXPIRE
→ RECOVER
```

---

# 199. AGENT PQC LOOP

```text
VECTOR
→ KEYGEN
→ SIGN
→ VERIFY
→ MUTATE
→ VERIFY FAIL
→ SERIALIZE
→ DESERIALIZE
→ VERIFY
→ FUZZ
→ SANITIZE
```

---

# 200. AGENT CONSENSUS LOOP

```text
SPEC VECTOR
→ IMPLEMENT
→ REFERENCE IMPLEMENTATION
→ COMPARE
→ VALID BLOCK
→ INVALID BLOCK
→ ADVERSARIAL BLOCK
→ FUZZ
→ REORG
→ RECOVERY
```

---

# 201. PROOF ARTIFACT RULE

Each phase should produce machine-readable evidence where useful.

Examples:

```text
proof.json
test-summary.json
benchmark.json
manifest.json
```

Human-readable companion:

```text
PROOF.md
```

---

# 202. PROOF FORMAT

Every proof should identify:

```text
claim
command
environment
source commit
input
expected
actual
result
timestamp
artifact
```

Do not record unverifiable "PASS" strings with no source.

---

# 203. SHA-VERIFIED RUNS

For important generated artifacts:

```text
input hash
output hash
tool hash/version
source commit
```

Record them.

This is especially important for:

- genesis
- state packs
- release builds
- test fixtures.

---

# 204. DETERMINISTIC GENERATORS

Any generator that influences protocol state should be deterministic from explicit input.

No hidden:

- current time
- local randomness
- machine identity
- environment variable
- network response

unless explicitly specified as part of the protocol.

---

# 205. RANDOMNESS POLICY

When randomness is required:

- use the appropriate cryptographic RNG,
- document why,
- do not use non-deterministic randomness in consensus calculations unless protocol-specified,
- write deterministic test hooks.

---

# 206. TIME POLICY

Consensus time uses protocol-defined semantics.

Tests must control timestamps deterministically.

Do not make consensus depend on wall-clock behavior outside the specified validation window.

---

# 207. FILE FORMAT VERSIONING

Every new persistent format requires:

```text
magic
version
length
integrity
compatibility rules
```

Examples:

```text
statepack v1
storage manifest v1
NFT manifest v1
```

---

# 208. MIGRATION POLICY

A format migration requires:

```text
old parser
→ backup
→ migration
→ validation
→ new parser
→ rollback path
```

---

# 209. NO DATA LOSS

For any migration, prove:

```text
before semantic state
==
after semantic state
```

for the supported dataset.

---

# 210. WALLET MIGRATION

Test:

- old wallet
- encrypted wallet
- descriptor wallet
- PQ wallet
- NFT-owned wallet
- storage-budget wallet.

---

# 211. NODE MODE MIGRATION

For:

```text
full → pruned
full → storage
compressed → normal
```

provide explicit migration semantics.

Do not change modes silently.

---

# 212. RPC COMPATIBILITY

If possible, preserve inherited safe RPC semantics.

Where changed:

- document,
- version,
- test.

---

# 213. API EXAMPLES

Each new RPC should have:

- request example,
- response example,
- errors,
- limits,
- authentication requirements,
- version.

Examples must be generated from real node behavior where possible.

---

# 214. OPENRPC / SCHEMA VALIDATION

If OpenRPC or JSON schema is used:

```text
schema
→ generate client/example
→ validate actual node response
```

Do not let docs drift from code.

---

# 215. EVENT DOCUMENTATION

Document:

- event name,
- trigger,
- payload,
- ordering expectations,
- replay behavior,
- version.

---

# 216. OBSERVABILITY

Add useful metrics for operators:

- current height
- sync progress
- peers
- validation time
- disk
- memory
- storage
- PQC
- reorgs.

Do not expose private user data.

---

# 217. OPTIONAL TELEMETRY

Any telemetry must be explicit, documented and privacy-conscious.

Default to local diagnostics.

Do not silently upload wallet or user data.

---

# 218. LOGGING

Use structured, useful logs.

Avoid:

- private keys
- huge raw payload dumps
- repetitive noise.

Important security events should include enough technical context to reproduce the problem.

---

# 219. ERROR RECOVERY UX

Every user-visible failure should answer:

```text
What happened?
Is my money/data safe?
What can I do?
```

Do not blame the user without evidence.

---

# 220. UI EMPTY STATES

Design real empty states:

```text
No wallet yet
No NFTs
No storage providers
No peers
Not synchronized
```

Do not show fake sample balances in production.

---

# 221. UI LOADING STATES

Provide:

- progress
- cancellation where safe
- status
- estimated workload when measurable.

Do not display made-up percentages.

---

# 222. UI SUCCESS STATES

Use actual backend confirmation.

Example:

```text
Broadcast accepted
```

must not be displayed as:

```text
Confirmed
```

until the node actually confirms it according to the UI's definition.

---

# 223. UI SECURITY LANGUAGE

Distinguish:

```text
Signed
Broadcast
Confirmed
Final/settled according to protocol
```

Do not collapse all states into "secure".

---

# 224. NODE HEALTH STATES

`Healthy` must be derived from real checks.

Examples:

```text
database valid
chain synced or synchronizing normally
P2P healthy
wallet accessible
PQC backend available
storage within configured limits
```

---

# 225. STORAGE HEALTH

Show separately:

```text
chain storage
state storage
NFT content cache
provider storage
```

---

# 226. NFT CONTENT HEALTH

Possible statuses:

```text
Available
Partially replicated
Degraded
Expired
Unpinned
Unknown
```

Do not infer global availability from a single local node.

---

# 227. PHASE EXIT CRITERIA

A phase ends only when:

1. implementation exists,
2. required tests exist,
3. required tests pass,
4. integration passes,
5. docs updated,
6. proof artifacts exist,
7. source audit passes,
8. checkpoint created.

For consensus phases, add:

9. differential/reference verification,
10. adversarial tests.

---

# 228. PHASE BLOCKER

If a phase cannot satisfy its exit criteria:

```text
PHASE = BLOCKED
```

Do not mark complete.

Independent non-dependent work may proceed.

---

# 229. MAINNET READINESS GATE

Required:

```text
Consensus PASS
Genesis PASS
PQC PASS
PoW PASS
Difficulty PASS
Wallet PASS
P2P PASS
Storage PASS
StatePack PASS
NFT PASS
RPC PASS
GUI PASS
Recovery PASS
Security PASS
Fuzz PASS
Sanitizer PASS
Build PASS
Installer PASS
Documentation PASS
External backup PASS
Release manifest PASS
```

---

# 230. ZERO KNOWN CRITICAL ISSUES

Before mainnet:

```text
0 known critical consensus bugs
0 known critical wallet-loss bugs
0 known critical cryptographic verification bugs
0 known critical release-integrity bugs
```

Known limitations may exist, but they must be documented and assessed.

---

# 231. TESTNET TO MAINNET

Do not jump from local regtest directly to mainnet.

Required path:

```text
regtest
→ private multi-node test
→ public testnet
→ long-run testnet
→ chaos testnet
→ release candidate
→ security review
→ genesis freeze
→ mainnet
```

---

# 232. MAINNET GENESIS FINALIZATION

Before genesis release:

```text
external backup
+
reproducible generation
+
independent verification
+
clean build
+
testnet validation
+
manifest freeze
```

Then lock the genesis for protocol v1.

---

# 233. FINAL PUSH CHECKLIST

Before final production push:

```text
[ ] External baseline backup
[ ] Final git bundle
[ ] Final source archive
[ ] Genesis backup
[ ] Chain manifest
[ ] Security evidence
[ ] Tests
[ ] Fuzz results
[ ] Sanitizer results
[ ] GUI QA
[ ] Installer QA
[ ] Secret scan
[ ] Large-file scan
[ ] Documentation audit
[ ] Reproducibility report
[ ] Artifact checksums
[ ] Release signature process ready
```

---

# 234. FINAL RELEASE CHECKLIST

```text
[ ] source reproducible
[ ] build reproducible where claimed
[ ] genesis reproducible
[ ] network identity unique
[ ] Bitcoin Mainnet rejected
[ ] Bitcoin datadir rejected
[ ] QuantBTC wallet isolated
[ ] PQC verified
[ ] SHA256d verified
[ ] difficulty verified
[ ] pruning verified
[ ] state pack lossless
[ ] NFT ownership verified
[ ] storage proof verified
[ ] storage rewards verified
[ ] automatic renewal bounded
[ ] API versioned
[ ] GUI functional
[ ] recovery verified
[ ] CI green
[ ] release artifacts hashed
[ ] README honest
[ ] SECURITY.md honest
```

---

# 235. RELEASE NOTES

Every release notes file must state:

- features
- protocol changes
- breaking changes
- migration
- security fixes
- known limitations
- build changes.

No marketing language may conceal a technical limitation.

---

# 236. FINAL README SECURITY SECTION

It must explicitly state:

```text
QuantBTC uses SHA256d Proof of Work.

QuantBTC does not claim zero generic quantum
search advantage against SHA256d.

QuantBTC uses post-quantum transaction
authorization according to its defined PQC
protocol.

QuantBTC does not mathematically eliminate
majority-PoW attacks.

External content availability depends on the
decentralized storage network and configured
replication.
```

Adapt exact wording only after the final protocol is fixed.

---

# 237. FINAL README BUILD SECTION

The final README must contain only commands that were actually validated against the final repository.

For each platform document:

```text
Prerequisites
Configure
Build
Test
Run
Wallet
Node
Release build
```

---

# 238. FINAL README OPERATOR SECTION

Explain:

```text
Full Node
Pruned Node
Archive Node
Storage Node
Mining
Backups
Recovery
Bandwidth
Disk
```

---

# 239. FINAL README DEVELOPER SECTION

Explain:

```text
Architecture
Consensus
PQC
State Packs
NFT
Storage
RPC
Testing
Release
```

---

# 240. FINAL README STATUS

At release time, use the actual status:

```text
Development
Testnet
Release Candidate
Mainnet
```

No false maturity claim.

---

# 241. FINAL SELF-HEALING CONTRACT

The agent is expected to repair failures itself where safe.

Safe self-healing includes:

- compiler errors,
- missing includes,
- schema mismatches,
- straightforward test bugs,
- deterministic serialization bugs,
- UI layout bugs,
- documentation drift,
- build configuration errors.

High-risk self-healing requires an explicit checkpoint and deeper verification:

- consensus
- cryptographic implementation
- wallet key management
- monetary logic
- storage reward logic
- NFT ownership logic.

---

# 242. HIGH-RISK CHANGE PROTOCOL

For a high-risk change:

```text
STOP
→ snapshot
→ write threat analysis
→ write or update tests
→ implement minimal change
→ run focused suite
→ run full related suite
→ run sanitizers
→ run fuzzing where relevant
→ differential test
→ document
→ checkpoint
```

---

# 243. RECOVERY FROM BAD PATCH

If a patch makes the working tree worse:

1. preserve the failure evidence,
2. revert only the bad change,
3. return to the last verified checkpoint,
4. do not rewrite unrelated work,
5. record why the approach failed,
6. try a different implementation strategy.

---

# 244. ALTERNATIVE IMPLEMENTATION PATHS

For difficult functionality, maintain at least two candidate strategies when practical.

Examples:

```text
PQC backend A / B
difficulty A / B
compression A / B
state-pack transport A / B
GUI implementation A / B
```

Prototype first.

Select based on:

- correctness
- auditability
- performance
- portability
- maintainability
- security.

Document rejected alternatives.

---

# 245. WEB RESEARCH FALLBACK PATH

If the preferred search tool is unavailable:

```text
official documentation
→ source repository
→ package manager metadata
→ local cached docs
→ primary paper
```

Never substitute a random blog for an official security specification when a primary source exists.

---

# 246. DEPENDENCY FAILURE FALLBACK

If a dependency cannot be built:

1. inspect supported versions,
2. inspect platform constraints,
3. test known compatible version,
4. consider a different production backend,
5. keep the security gate blocked if compatibility is not proven.

Never silently substitute a weaker/unreviewed crypto implementation.

---

# 247. NETWORK FAILURE FALLBACK

If network access is unavailable:

- use local source,
- use cached dependencies,
- use previously downloaded artifacts,
- continue deterministic local work.

But mark web-dependent verification as:

```text
NOT VERIFIED ONLINE
```

and keep release gates requiring current external verification blocked.

---

# 248. GUI TOOL FAILURE FALLBACK

If automated GUI tooling is unavailable:

- build and launch manually where possible,
- capture screenshots,
- perform structured manual QA,
- record missing automation coverage.

Do not claim automated visual QA passed if it did not run.

---

# 249. BUILD TOOL FAILURE FALLBACK

If the primary compiler fails:

- inspect whether the failure is code or environment,
- test a supported alternate compiler if relevant,
- preserve target-platform release gates.

A successful Linux build does not prove a Windows build.

---

# 250. RELEASE FAIL-SAFE

If any critical release gate cannot be verified:

```text
RELEASE = BLOCKED
```

The agent may prepare documentation and diagnostics, but must not label the release production-ready.

---

# 251. AGENT COMMUNICATION STYLE

When writing reports:

Prefer:

```text
PASS
FAIL
BLOCKED
NOT RUN
NOT VERIFIED
KNOWN LIMITATION
```

Avoid vague:

```text
looks good
probably fixed
should work
seems secure
```

unless clearly marked as an assessment rather than evidence.

---

# 252. DAILY / ITERATION SUMMARY

After a meaningful work cycle, update:

```text
.agent/STATE.md
```

with:

```text
Current phase
Completed
Tests
Failures
Open blockers
Next deterministic action
Last checkpoint
Last verified commit
```

---

# 253. NO REPETITIVE WASTE

If an approach fails repeatedly, stop repeating it.

After repeated identical failure:

```text
attempt 1
attempt 2
attempt 3
```

and same root cause remains:

```text
BLOCKED / CHANGE STRATEGY
```

Research an alternative or isolate the issue.

---

# 254. END-OF-PHASE SUMMARY

Every phase must create:

```text
.agent/PROOFS/<phase>/summary.md
```

and where appropriate:

```text
summary.json
```

Include:

```text
scope
changes
tests
proofs
known limitations
rollback
next phase
```

---

# 255. MAIN AGENT COMMANDMENT

The agent must always prefer:

```text
verified small progress
```

over:

```text
unverified large progress
```

---

# 256. FINAL ZERO-TO-HERO EXECUTION PLAN

Execute this exact macro-flow:

```text
START
↓
locate repo
↓
verify git state
↓
backup outside repo
↓
verify backup
↓
baseline build
↓
baseline tests
↓
audit repository
↓
map Bitcoin identity
↓
create QuantBTC architecture/spec
↓
chain identity
↓
genesis generator
↓
new genesis
↓
consensus isolation
↓
SHA256d validation
↓
difficulty
↓
PQC abstraction
↓
PQC signatures
↓
wallet
↓
pruning
↓
state packs
↓
storage protocol
↓
NFT
↓
storage economy
↓
RPC
↓
P2P
↓
GUI
↓
cross-chain verification
↓
full test system
↓
fuzz/sanitizers
↓
visual QA
↓
testnet
↓
chaos / long-run
↓
security review
↓
release candidate
↓
external backup
↓
genesis/manifest freeze
↓
clean build
↓
reproducibility
↓
package
↓
checksum
↓
sign
↓
push/tag
↓
remote CI
↓
verify release
↓
MAINNET
```

---

# 257. FIRST SESSION — WHAT TO DO NOW

When this file is first loaded, do NOT begin by writing NFT code.

Perform:

```text
1. Identify repository root.
2. Read git status.
3. Read current HEAD and branch.
4. Read this GEMINI.md fully.
5. Create external baseline backup.
6. Verify backup.
7. Map repository structure.
8. Compare current fork with upstream Bitcoin Core.
9. Locate current chainparams, PoW, wallet, node, RPC, Qt, test and build systems.
10. Build the untouched baseline.
11. Run baseline tests.
12. Generate repository audit.
13. Create or update .agent/STATE.md.
14. Create protocol specification skeleton.
15. Only then begin Phase 1 implementation.
```

---

# 258. FIRST FEATURE IMPLEMENTATION

The first implementation task is:

```text
QuantBTC chain identity
```

not:

```text
NFT
```

not:

```text
marketplace
```

not:

```text
GUI cosmetics
```

The first implementation must establish the independent chain foundation.

---

# 259. FIRST CHAIN IDENTITY DELIVERABLE

Produce:

```text
chain_id
network magic
ports
datadir identity
address-prefix placeholders
seed strategy
binary naming strategy
chainparams isolation
tests
documentation
```

Do not freeze values until the protocol design is complete.

---

# 260. NEXT STEP AFTER CHAIN IDENTITY

Implement the deterministic genesis generator.

The agent must not hand-edit a guessed genesis hash.

---

# 261. AFTER GENESIS

Run:

```text
clean build
unit tests
genesis tests
chain isolation tests
cross-network rejection
```

Then checkpoint.

---

# 262. AFTER CONSENSUS

Run:

```text
full consensus suite
property tests
fuzzing
differential reference tests
```

Then checkpoint.

---

# 263. AFTER PQC

Run:

```text
PQC vectors
wallet signing
wallet restore
invalid-signature tests
wrong-algorithm tests
serialization tests
fuzz
sanitizers
platform matrix
```

Then checkpoint.

---

# 264. AFTER STORAGE / STATEPACK

Run:

```text
real state export
real compression
real decompression
hash equality
corruption
partial file
wrong-chain import
fallback to normal sync
```

Then checkpoint.

---

# 265. AFTER NFT

Run:

```text
real mint
real transfer
real ownership verification
storage lease
renewal
failed payment
expiration
reorg
provider failure
```

Then checkpoint.

---

# 266. AFTER GUI

Run:

```text
real node
real wallet
real transaction
real NFT
real storage
real diagnostics
real restart
visual QA
```

Then checkpoint.

---

# 267. AFTER TESTNET

Run:

```text
multi-node
long-run
chaos
reorg
PQC
NFT
storage
recovery
```

Then release candidate.

---

# 268. FINAL STOP CONDITIONS

STOP immediately and preserve evidence if:

- genesis is ambiguous,
- consensus differs between implementations,
- PQ verification disagrees,
- state root differs after decompression,
- wallet restore loses state,
- reward accounting overflows,
- security-critical dependency cannot be verified,
- release build cannot be reproduced within the claimed scope,
- secret is found,
- test result is contradictory,
- remote CI disagrees with local results.

---

# 269. FINAL "DONE" DEFINITION

A feature is DONE only when:

```text
SPEC
+
IMPLEMENTATION
+
UNIT TEST
+
INTEGRATION
+
FUNCTIONAL
+
SECURITY
+
RECOVERY
+
DOCUMENTATION
+
PROOF
```

all agree.

---

# 270. FINAL "MAINNET READY" DEFINITION

Mainnet readiness requires:

```text
all mandatory protocol features implemented
all critical tests passing
critical fuzz/sanitizer coverage completed
wallet restore verified
genesis independently reproduced
chain identity isolated
PQC verified
PoW verified
difficulty stress-tested
state packs proven lossless
storage proofs verified
NFT state reorg-safe
RPC documented and tested
GUI behavior verified
cross-platform builds validated
installer validated
release artifacts hashed
external backups verified
final source audit completed
README and SECURITY.md are honest
```

---

# 271. FINAL HUMAN-READABLE RESULT

At the end, the repository should answer three questions clearly.

## What is QuantBTC?

A standalone blockchain based on a Bitcoin-Core engineering foundation, with its own chain identity and protocol.

## What is it protected against?

Its defined threat model must say exactly which classical and post-quantum attacks the protocol addresses.

## What does it NOT guarantee?

It must clearly disclose:

- generic quantum search considerations for SHA256d,
- majority-PoW limitations,
- dependency risks,
- implementation risks,
- decentralized storage availability limitations.

---

# 272. MASTER RULE

When uncertain:

```text
INSPECT
→ RESEARCH
→ TEST
→ VERIFY
→ DOCUMENT
```

When broken:

```text
REPRODUCE
→ MINIMIZE
→ FIX
→ REGRESSION TEST
→ VERIFY
```

When ready:

```text
BACKUP
→ AUDIT
→ COMMIT
→ PUSH
→ VERIFY CI
```

When not ready:

```text
STOP
→ DOCUMENT
→ FIX
```

---

# 273. START NOW

**Do not ask for a planning confirmation.**

Begin by executing the FIRST SESSION procedure from section 257.

Do not skip the external backup.

Do not skip the untouched baseline build.

Do not skip the repository audit.

Do not skip the test baseline.

Do not implement features from memory when the repository can answer the question directly.

Do not claim completion until the required evidence exists.

**Start.**
