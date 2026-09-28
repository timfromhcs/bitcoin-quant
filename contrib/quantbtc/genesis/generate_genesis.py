"""Deterministic QuantBTC genesis-block generator (§30).

Reads genesis-spec.json, builds the coinbase transaction, derives the merkle
root, mines the nonce by bounded search (deterministic: nonce 0,1,2,...),
and writes:
  generated/genesis-manifest.json   (chain params + hashes, §32)
  generated/genesis-block.hex        (raw 80-byte header hex)
  generated/genesis-tx.hex           (raw coinbase tx hex)
  vectors/genesis-vectors.json       (serialization test vector)

Determinism (§204): all inputs come from genesis-spec.json; no wall-clock,
no randomness, no machine identity. Bounded loop: MAX_NONCE attempts, then
deterministic FAIL (never an unbounded `while True`, §12).
"""
import hashlib
import json
import struct
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent / "ref"))
from pow import sha256d, serialize_header, check_pow, compact_to_target  # noqa: E402

MAX_NONCE = 2**32  # bounded search space; generator FAILs deterministically if exceeded


def varint(n: int) -> bytes:
    if n < 0xFD:
        return struct.pack("B", n)
    if n <= 0xFFFF:
        return b"\xfd" + struct.pack("<H", n)
    if n <= 0xFFFFFFFF:
        return b"\xfe" + struct.pack("<I", n)
    return b"\xff" + struct.pack("<Q", n)


def build_coinbase(spec: dict) -> bytes:
    """Coinbase tx mirroring CBlock creation in src/kernel/chainparams.cpp:
    scriptSig = push(bits_be4) + push(height_num) + push(timestamp_msg)."""
    version = struct.pack("<i", 1)
    prev = b"\x00" * 32 + struct.pack("<I", 0xFFFFFFFF)
    bits_bytes = bytes.fromhex(spec["coinbase_bits_push"])          # e.g. ffff001d
    height_byte = bytes([spec["coinbase_height_push"]])             # e.g. 04
    msg = spec["timestamp_message"].encode("ascii")
    assert all(c < 128 for c in msg), "timestamp message must be ASCII"
    scriptsig = (bytes([len(bits_bytes)]) + bits_bytes
                 + b"\x01" + height_byte
                 + bytes([len(msg)]) + msg)
    txin = prev + varint(len(scriptsig)) + scriptsig + struct.pack("<I", 0xFFFFFFFF)
    value = struct.pack("<q", spec["reward_satoshis"])
    pkscript = bytes.fromhex(spec["genesis_pubkey_script"])
    txout = value + varint(len(pkscript)) + pkscript
    return version + varint(1) + txin + varint(1) + txout + struct.pack("<I", 0)


def main() -> int:
    spec = json.loads((HERE / "genesis-spec.json").read_text())
    coinbase = build_coinbase(spec)
    txid_le = sha256d(coinbase)                      # single tx -> merkle root = txid
    merkle_root_le = txid_le
    prev_le = b"\x00" * 32
    nbits = int(spec["nbits"], 16)
    target, neg, ovf = compact_to_target(nbits)
    assert not neg and not ovf and target > 0, "spec nBits must encode a valid target"

    found = None
    for nonce in range(MAX_NONCE):
        header = serialize_header(spec["version"], prev_le, merkle_root_le,
                                  spec["timestamp"], nbits, nonce)
        ok, _ = check_pow(header, nbits)
        if ok:
            found = (nonce, header)
            break
    if found is None:
        print("FAIL: nonce space exhausted without meeting target", file=sys.stderr)
        return 1
    nonce, header = found
    block_hash_le = sha256d(header)

    def be(b: bytes) -> str:  # internal -> display order
        return b[::-1].hex()

    manifest = {
        "chain_id": spec["chain_id"],
        "protocol_version": spec["protocol_version"],
        "status": spec["status"],
        "timestamp": spec["timestamp"],
        "version": spec["version"],
        "nbits": spec["nbits"],
        "nonce": nonce,
        "reward_satoshis": spec["reward_satoshis"],
        "timestamp_message": spec["timestamp_message"],
        "tx_hash": be(txid_le),
        "merkle_root": be(merkle_root_le),
        "genesis_hash": be(block_hash_le),
        "generator": "contrib/quantbtc/genesis/generate_genesis.py",
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
    print(json.dumps({"result": "PASS", "nonce": nonce,
                      "genesis_hash": be(block_hash_le)}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
