"""Storage proof-of-retrievability reference (§78-§81, §114).

PDP-lite with Merkle spot-checks (Bitcoin duplicate-odd tree semantics):
  leaf(i)   = SHA256d(chunk_i)
  root      = merkle over leaves (odd node duplicated, like consensus/merkle)
  challenge = SHA256d(tip_hash || provider_id || BE64(epoch)) -> k indices
  proof     = per-sample (chunk bytes, sibling path bottom-up)
  verify    = fold leaf hash through path, compare to committed root.

Deterministic, stdlib-only. Honest scope: spot-checks prove retrievability
of the SAMPLED chunks, not permanent full replication (see spec §12).
"""
import hashlib
import struct

CHUNK_SIZE = 65536
SAMPLES = 16


def sha256d(b: bytes) -> bytes:
    return hashlib.sha256(hashlib.sha256(b).digest()).digest()


def chunkify(data: bytes, size: int = CHUNK_SIZE) -> list:
    return [data[i:i + size] for i in range(0, max(len(data), 1), size)]


def merkle_root(leaves: list) -> bytes:
    """Bitcoin duplicate-odd semantics (matches consensus/merkle.cpp)."""
    assert leaves, "empty content has no root"
    level = list(leaves)
    while len(level) > 1:
        if len(level) % 2 == 1:
            level.append(level[-1])
        level = [sha256d(level[i] + level[i + 1]) for i in range(0, len(level), 2)]
    return level[0]


def content_commit(data: bytes) -> tuple:
    """Returns (chunk_hashes, root)."""
    chunks = chunkify(data)
    hashes = [sha256d(c) for c in chunks]
    return hashes, merkle_root(hashes)


def derive_indices(root: bytes, provider_id: bytes, epoch: int,
                   n_leaves: int, k: int = SAMPLES) -> list:
    """Protocol challenge entropy (§80): chain tip root binds content,
    provider id binds the prover, epoch binds time. Provider cannot grind."""
    assert len(provider_id) == 32
    seed = sha256d(root + provider_id + struct.pack(">Q", epoch))
    out = []
    for i in range(k):
        h = sha256d(seed + struct.pack(">I", i))
        out.append(int.from_bytes(h, "big") % n_leaves)
    return out


def prove(chunks: list, index: int) -> dict:
    """Merkle inclusion proof for leaf `index` (siblings bottom-up)."""
    level = [sha256d(c) for c in chunks]
    path = []
    idx = index
    while len(level) > 1:
        if len(level) % 2 == 1:
            level.append(level[-1])
        path.append(level[idx ^ 1])
        level = [sha256d(level[i] + level[i + 1]) for i in range(0, len(level), 2)]
        idx //= 2
    return {"index": index, "chunk": chunks[index], "path": path}


def verify(root: bytes, proof: dict) -> tuple:
    """Returns (ok, reason). Deterministic, no secrets."""
    try:
        idx, chunk, path = proof["index"], proof["chunk"], proof["path"]
    except (KeyError, TypeError):
        return False, "malformed-proof"
    if not isinstance(idx, int) or idx < 0 or not isinstance(chunk, (bytes, bytearray)):
        return False, "malformed-proof"
    if len(chunk) == 0 or len(chunk) > CHUNK_SIZE:
        return False, "bad-chunk-size"
    h = sha256d(bytes(chunk))
    for sib in path:
        if not isinstance(sib, (bytes, bytearray)) or len(sib) != 32:
            return False, "bad-path"
        sib = bytes(sib)
        h = sha256d(h + sib) if idx % 2 == 0 else sha256d(sib + h)
        idx //= 2
    if idx != 0:
        return False, "path-length-mismatch"
    if h != root:
        return False, "root-mismatch"
    return True, "ok"


def encode_proof(proof: dict) -> bytes:
    """Proof envelope v1 (§207): ver || idx LE32 || pathlen LE32 ||
    siblings || chunklen LE32 || chunk. Strict, versioned."""
    out = bytes([1]) + struct.pack("<I", proof["index"])
    out += struct.pack("<I", len(proof["path"]))
    for sib in proof["path"]:
        assert len(sib) == 32
        out += bytes(sib)
    out += struct.pack("<I", len(proof["chunk"])) + bytes(proof["chunk"])
    return out


def decode_proof(blob: bytes) -> tuple:
    if len(blob) < 13:
        return None, "too-short"
    if blob[0] != 1:
        return None, "bad-version"
    idx = struct.unpack("<I", blob[1:5])[0]
    npath = struct.unpack("<I", blob[5:9])[0]
    pos = 9
    if len(blob) < pos + 32 * npath + 4:
        return None, "truncated"
    path = [blob[pos + 32 * i:pos + 32 * (i + 1)] for i in range(npath)]
    pos += 32 * npath
    clen = struct.unpack("<I", blob[pos:pos + 4])[0]
    pos += 4
    if clen == 0 or clen > CHUNK_SIZE or len(blob) != pos + clen:
        return None, "bad-chunk-size"
    return {"index": idx, "chunk": blob[pos:], "path": path}, "ok"


def detection_probability(missing_frac: float, k: int = SAMPLES) -> float:
    """P(at least one challenged chunk missing) = 1-(1-p)^k."""
    return 1.0 - (1.0 - missing_frac) ** k


def _selftest():
    import random
    rng = random.Random(0x5702A6E)
    # sizes incl. edge cases: 1 byte, exact chunk, chunk+1, multi + odd leaves
    for size in (1, CHUNK_SIZE - 1, CHUNK_SIZE, CHUNK_SIZE + 1, 3 * CHUNK_SIZE + 17):
        data = bytes(rng.randrange(256) for _ in range(size))
        hashes, root = content_commit(data)
        chunks = chunkify(data)
        assert len(hashes) == len(chunks)
        pid = sha256d(b"provider-1")
        for epoch in (0, 1, 2**64 - 1):
            for idx in derive_indices(root, pid, epoch, len(chunks)):
                pr = prove(chunks, idx)
                ok, reason = verify(root, pr)
                assert ok, (size, epoch, idx, reason)
                # envelope round-trip
                dec, why = decode_proof(encode_proof(pr))
                assert dec is not None, why
                ok2, _ = verify(root, dec)
                assert ok2
        # negatives
        pr = prove(chunks, 0)
        bad = dict(pr)
        bad["chunk"] = bytes(b ^ 1 for b in bytes([pr["chunk"][0]])) + pr["chunk"][1:] \
            if len(pr["chunk"]) else b"\x00"
        assert verify(root, bad) == (False, "root-mismatch")
        bad2 = dict(pr, path=[b"\x00" * 32] + pr["path"][1:] if pr["path"] else [])
        if pr["path"]:
            assert not verify(root, bad2)[0]
        assert verify(b"\x00" * 32, pr) == (False, "root-mismatch")
        assert decode_proof(b"")[1] == "too-short"
        assert decode_proof(bytes([2]) + b"\x00" * 12)[1] == "bad-version"
        assert decode_proof(encode_proof(pr)[:-1])[1] in ("truncated", "bad-chunk-size")
    # determinism: same inputs twice
    data = bytes(rng.randrange(256) for _ in range(100000))
    _, root = content_commit(data)
    pid = sha256d(b"p")
    assert derive_indices(root, pid, 7, 2) == derive_indices(root, pid, 7, 2)
    # detection curve sanity
    assert abs(detection_probability(0.01) - (1 - 0.99 ** 16)) < 1e-12
    assert detection_probability(0.5) > 0.9999
    print("storage proof self-test: PASS")


if __name__ == "__main__":
    _selftest()
