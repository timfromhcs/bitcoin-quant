"""Deterministic QuantBTC genesis-block generator (§30). FINAL protocol v1.

Reads genesis-spec.json, derives the NUMS genesis pubkey (deterministic
secp256k1 point grinding, no trusted ceremony), builds the coinbase
transaction, derives the merkle root, mines the nonce by bounded search, and
writes:
  generated/genesis-manifest.json   (chain params + hashes, §32)
  generated/genesis-block.hex        (raw 80-byte header hex)
  generated/genesis-tx.hex           (raw coinbase tx hex)
  vectors/genesis-vectors.json       (serialization test vector)

Mining note: a full 32-bit nonce space at difficulty 1 holds ~1 expected
solution (P(empty) ~= 37%). If the space is empty, the generator rolls the
timestamp forward by whole seconds (bounded by MAX_ROLL, deterministic FAIL
after) and mines the next space. The final timestamp + roll are recorded in
the manifest, so the same spec always reproduces the same genesis.

Determinism (§204): within one timestamp the result is the global minimum
nonce of the partitioned search over a FIXED worker count from the spec,
which is scheduling-independent. Same spec -> same result on any machine.
No wall-clock, no randomness, no machine identity enters the result.
"""
import hashlib
import json
import multiprocessing as mp
import struct
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent / "ref"))
from pow import sha256d, serialize_header, check_pow, compact_to_target  # noqa: E402

MAX_NONCE = 2**32
MAX_ROLL = 3600  # max timestamp-roll seconds; deterministic FAIL after
SECP_P = 2**256 - 2**32 - 977


def nums_pubkey(label: str) -> bytes:
    """Deterministic compressed pubkey: smallest counter giving a valid
    secp256k1 point for x = SHA256(label || BE32(counter)). Verifiable by
    anyone; no trusted key ceremony. The genesis output is unspendable by
    consensus rule regardless of key knowledge."""
    counter = 0
    while True:
        x = int.from_bytes(hashlib.sha256(
            label.encode("ascii") + struct.pack(">I", counter)).digest(), "big") % SECP_P
        rhs = (pow(x, 3, SECP_P) + 7) % SECP_P
        y = pow(rhs, (SECP_P + 1) // 4, SECP_P)
        if (y * y) % SECP_P == rhs:
            return bytes([0x02 | (y & 1)]) + x.to_bytes(32, "big")
        counter += 1
        assert counter < 1000, "NUMS grinding unexpectedly hard"


def varint(n: int) -> bytes:
    if n < 0xFD:
        return struct.pack("B", n)
    if n <= 0xFFFF:
        return b"\xfd" + struct.pack("<H", n)
    if n <= 0xFFFFFFFF:
        return b"\xfe" + struct.pack("<I", n)
    return b"\xff" + struct.pack("<Q", n)


def build_coinbase(spec: dict, pubkey_script: bytes) -> bytes:
    """Coinbase tx mirroring CBlock creation in src/kernel/chainparams.cpp:
    scriptSig = push(bits_be4) + push(height_num) + push(timestamp_msg)."""
    version = struct.pack("<i", 1)
    prev = b"\x00" * 32 + struct.pack("<I", 0xFFFFFFFF)
    bits_bytes = bytes.fromhex(spec["coinbase_bits_push"])
    height_byte = bytes([spec["coinbase_height_push"]])
    msg = spec["timestamp_message"].encode("ascii")
    assert all(c < 128 for c in msg), "timestamp message must be ASCII"
    scriptsig = (bytes([len(bits_bytes)]) + bits_bytes
                 + b"\x01" + height_byte
                 + bytes([len(msg)]) + msg)
    assert len(scriptsig) <= 100, "coinbase scriptsig exceeds 100 bytes"
    txin = prev + varint(len(scriptsig)) + scriptsig + struct.pack("<I", 0xFFFFFFFF)
    value = struct.pack("<q", spec["reward_satoshis"])
    txout = value + varint(len(pubkey_script)) + pubkey_script
    return version + varint(1) + txin + varint(1) + txout + struct.pack("<I", 0)


def _worker_ledger(w, prefix, target, state):
    """Scan ledger chunks; record per-chunk minimum hit nonce."""
    import hashlib as _hl
    import struct as _st
    sha = _hl.sha256
    lock, next_idx, done_flags, hits, total, csize = state
    while True:
        with lock:
            idx = next_idx.value
            while idx < total and done_flags[idx]:
                idx += 1
            if idx >= total:
                return
            next_idx.value = idx + 1
        lo = idx * csize
        best_n = MAX_NONCE
        tb = target.to_bytes(32, "big")
        for n in range(lo, min(lo + csize, MAX_NONCE)):
            d = sha(sha(prefix + _st.pack("<I", n)).digest()).digest()
            if d[::-1] <= tb and n < best_n:
                best_n = n
                if best_n == lo:
                    break  # cannot beat chunk minimum
        with lock:
            done_flags[idx] = 1
            if best_n < hits[idx]:
                hits[idx] = best_n


def mine(prefix: bytes, target: int, workers: int, progress_path) -> tuple:
    """Chunk-ledger parallel search. Deterministic result = hit from the
    lowest-indexed chunk containing one (scheduling-independent). Resumable
    via sidecar file. Returns (nonce, chunks_scanned). Raises on exhaustion."""
    import time as _time
    total = 256
    csize = MAX_NONCE // total
    lock = mp.Lock()
    next_idx = mp.Value("i", 0)
    done_flags = mp.Array("b", total)
    hits = mp.Array("Q", [MAX_NONCE] * total)
    completed = set()
    if progress_path.exists():
        try:
            saved = json.loads(progress_path.read_text())
            for i in saved.get("completed", []):
                if 0 <= i < total:
                    done_flags[i] = 1
                    completed.add(i)
            for k, v in saved.get("hits", {}).items():
                if 0 <= int(k) < total and v < hits[int(k)]:
                    hits[int(k)] = v
        except (ValueError, OSError):
            pass
    state = (lock, next_idx, done_flags, hits, total, csize)
    procs = [mp.Process(target=_worker_ledger, args=(w, prefix, target, state))
             for w in range(workers)]
    t0 = _time.time()
    for pr in procs:
        pr.start()
    while any(pr.is_alive() for pr in procs):
        for pr in procs:
            pr.join(timeout=30)
        done = sum(done_flags)
        el = _time.time() - t0
        rate = (done * csize) / max(el, 0.1)
        print("mining: %d/%d chunks (%.1f%%) elapsed %.0fs rate %.2f MH/s"
              % (done, total, 100.0 * done / total, el, rate / 1e6), flush=True)
        with lock:
            snap = {"completed": [i for i in range(total) if done_flags[i]],
                    "hits": {str(i): hits[i] for i in range(total)
                             if hits[i] < MAX_NONCE}}
        progress_path.write_text(json.dumps(snap))
        if done >= total:
            break
    for pr in procs:
        pr.join()
    for i in range(total):
        if hits[i] < MAX_NONCE:
            try:
                progress_path.unlink()
            except OSError:
                pass
            return hits[i], total
    try:
        progress_path.unlink()
    except OSError:
        pass
    raise RuntimeError("nonce space exhausted without meeting target")


def git_head() -> str:
    try:
        out = subprocess.run(["git", "rev-parse", "--short", "HEAD"],
                             capture_output=True, text=True, cwd=str(HERE.parents[2]))
        dirty = subprocess.run(["git", "status", "--short"],
                               capture_output=True, text=True, cwd=str(HERE.parents[2]))
        head = out.stdout.strip() or "UNKNOWN"
        return head + ("-dirty" if dirty.stdout.strip() else "")
    except Exception:
        return "UNKNOWN"


def main() -> int:
    spec = json.loads((HERE / "genesis-spec.json").read_text())
    pubkey = nums_pubkey(spec["genesis_pubkey_nums_label"])
    # P2PK: push(len) + key + OP_CHECKSIG. The push opcode MUST match the key
    # length (33 = 0x21 for compressed keys); a stale 0x41 here once caused
    # a generator/consensus divergence (caught by the C++ genesis assert).
    pubkey_script = bytes([len(pubkey)]) + pubkey + b"\xac"
    coinbase = build_coinbase(spec, pubkey_script)
    txid_le = sha256d(coinbase)                      # single tx -> merkle root = txid
    merkle_root_le = txid_le
    prev_le = b"\x00" * 32
    nbits = int(spec["nbits"], 16)
    target, neg, ovf = compact_to_target(nbits)
    assert not neg and not ovf and target > 0, "spec nBits must encode a valid target"

    prefix_base = (struct.pack("<i", spec["version"]) + prev_le + merkle_root_le)
    spec_digest = hashlib.sha256(
        json.dumps(spec, sort_keys=True).encode()).hexdigest()[:16]
    progress_path = (HERE / "generated" /
                     ("mining-progress-%s.json" % spec_digest))
    timestamp = None
    nonce = None
    for roll in range(MAX_ROLL + 1):
        ts = spec["timestamp"] + roll
        prefix = prefix_base + struct.pack("<II", ts, nbits)
        try:
            nonce, _chunks = mine(prefix, target, int(spec["mine_workers"]),
                                  progress_path)
        except RuntimeError:
            print("nonce space empty at timestamp %d (roll %d), continuing" % (ts, roll))
            try:
                progress_path.unlink()
            except OSError:
                pass
            continue
        timestamp = ts
        break
    if nonce is None or timestamp is None:
        print("FAIL: no solution within MAX_ROLL seconds", file=sys.stderr)
        return 1
    header = prefix + struct.pack("<I", nonce)
    ok, reason = check_pow(header, nbits)
    assert ok, reason
    block_hash_le = sha256d(header)

    def be(b: bytes) -> str:  # internal -> display order
        return b[::-1].hex()

    manifest = {
        "chain_id": spec["chain_id"],
        "protocol_version": spec["protocol_version"],
        "status": spec["status"],
        "timestamp": timestamp,
        "timestamp_base": spec["timestamp"],
        "timestamp_roll": timestamp - spec["timestamp"],
        "version": spec["version"],
        "nbits": spec["nbits"],
        "nonce": nonce,
        "mine_workers": spec["mine_workers"],
        "reward_satoshis": spec["reward_satoshis"],
        "timestamp_message": spec["timestamp_message"],
        "genesis_pubkey_nums_label": spec["genesis_pubkey_nums_label"],
        "genesis_pubkey_compressed": pubkey.hex(),
        "tx_hash": be(txid_le),
        "merkle_root": be(merkle_root_le),
        "genesis_hash": be(block_hash_le),
        "generator": "contrib/quantbtc/genesis/generate_genesis.py",
        "source_commit": git_head(),
    }
    gen = HERE / "generated"
    gen.mkdir(exist_ok=True)
    (gen / "genesis-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    (gen / "genesis-block.hex").write_text(header.hex() + "\n")
    (gen / "genesis-tx.hex").write_text(coinbase.hex() + "\n")
    vectors = {
        "header_hex": header.hex(),
        "tx_hex": coinbase.hex(),
        "expected_txid": be(txid_le),
        "expected_merkle_root": be(merkle_root_le),
        "expected_genesis_hash": be(block_hash_le),
    }
    (HERE / "vectors" / "genesis-vectors.json").write_text(json.dumps(vectors, indent=2) + "\n")
    print(json.dumps({"result": "PASS", "nonce": nonce, "timestamp": timestamp,
                      "genesis_hash": be(block_hash_le)}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
