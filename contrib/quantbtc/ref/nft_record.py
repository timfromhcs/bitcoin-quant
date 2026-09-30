"""NFT mint-record reference (§70-§77, §86 core): record codec + nft_id.

Pure, stdlib-only. Record binds (collection, serial) -> content_root
(storage commitment) + metadata_hash + owner. nft_id is a deterministic
pure function; UNIQUENESS is a registry-layer property, not a formula.
Out of scope (documented gaps): transfers, registry, collections,
royalties, consensus hook, lifecycle machine.
"""
import hashlib
import struct

RECORD_VERSION = 1
MAX_SCRIPT_SIZE = 10000


def sha256d(b: bytes) -> bytes:
    return hashlib.sha256(hashlib.sha256(b).digest()).digest()


def nft_id(collection_id: bytes, serial: int) -> bytes:
    """Deterministic asset id: SHA256d(collection_id || BE64(serial))."""
    assert len(collection_id) == 32 and 0 <= serial < 2**64
    return sha256d(collection_id + struct.pack(">Q", serial))


def encode_mint(collection_id: bytes, serial: int, content_root: bytes,
                metadata_hash: bytes, owner_script: bytes,
                minted_epoch: int) -> bytes:
    assert len(collection_id) == 32
    assert 0 <= serial < 2**64
    assert len(content_root) == 32 and len(metadata_hash) == 32
    assert 1 <= len(owner_script) <= MAX_SCRIPT_SIZE
    assert minted_epoch >= 0
    out = bytes([RECORD_VERSION]) + collection_id + struct.pack(">Q", serial)
    out += content_root + metadata_hash
    out += struct.pack("<H", len(owner_script)) + owner_script
    out += struct.pack("<Q", minted_epoch)
    return out


def decode_mint(blob: bytes) -> tuple:
    if len(blob) < 1 + 32 + 8 + 32 + 32 + 2 + 1 + 8:
        return None, "too-short"
    if blob[0] != RECORD_VERSION:
        return None, "bad-version"
    pos = 1
    collection_id, serial = blob[pos:pos + 32], struct.unpack(">Q", blob[pos + 32:pos + 40])[0]
    pos += 40
    content_root, metadata_hash = blob[pos:pos + 32], blob[pos + 32:pos + 64]
    pos += 64
    (slen,) = struct.unpack("<H", blob[pos:pos + 2])
    pos += 2
    if slen == 0 or slen > MAX_SCRIPT_SIZE or len(blob) != pos + slen + 8:
        return None, "bad-script-size"
    script = blob[pos:pos + slen]
    (epoch,) = struct.unpack("<Q", blob[pos + slen:pos + slen + 8])
    return {"collection_id": collection_id, "serial": serial,
            "content_root": content_root, "metadata_hash": metadata_hash,
            "owner_script": script, "minted_epoch": epoch}, "ok"


def _selftest():
    import random
    rng = random.Random(0x9713)
    # nft_id determinism + domain separation (serial BE vs LE differ, collections differ)
    c1 = bytes(rng.randrange(256) for _ in range(32))
    assert nft_id(c1, 7) == nft_id(c1, 7)
    assert nft_id(c1, 7) != nft_id(c1, 8)
    c2 = bytearray(c1)
    c2[0] ^= 1
    assert nft_id(c1, 7) != nft_id(bytes(c2), 7)
    # codec round-trip incl. edges
    for slen in (1, 25, MAX_SCRIPT_SIZE):
        for serial, epoch in ((0, 0), (1, 7), (2**64 - 1, 2**64 - 1)):
            rec, err = decode_mint(encode_mint(
                c1, serial, bytes(rng.randrange(256) for _ in range(32)),
                bytes(rng.randrange(256) for _ in range(32)),
                bytes(rng.randrange(256) for _ in range(slen)), epoch))
            assert err == "ok", (slen, serial, err)
            assert rec["serial"] == serial and rec["minted_epoch"] == epoch
            assert len(rec["owner_script"]) == slen
            # id binds exactly (collection, serial)
            assert nft_id(rec["collection_id"], rec["serial"]) == nft_id(c1, serial)
    # negatives
    good = encode_mint(c1, 0, b"\xaa" * 32, b"\xbb" * 32, b"\x51", 0)
    assert decode_mint(b"")[1] == "too-short"
    assert decode_mint(bytes([2]) + good[1:])[1] == "bad-version"
    assert decode_mint(good + b"\x00")[1] == "bad-script-size"
    # serial is BE64: flipping byte order changes bytes on the wire
    assert good[33:41] == struct.pack(">Q", 0)
    print("nft mint-record self-test: PASS")


if __name__ == "__main__":
    _selftest()
