"""QuantBTC reference PoW primitives (differential-testing reference, §114).

Pure-stdlib second implementation of:
  - SHA256d block-header hashing (must match src/crypto/sha256 + src/primitives/block)
  - compact (nBits) -> target conversion incl. negative/overflow rules
    (must match src/arith_uint256 SetCompact + CheckProofOfWork in src/pow.cpp)
  - header serialization (80 bytes, little-endian)

No consensus values are frozen here; this module is consensus-parameter-free.
"""
import hashlib
import struct


def sha256d(data: bytes) -> bytes:
    """Double SHA256, returns raw 32 bytes (internal byte order)."""
    return hashlib.sha256(hashlib.sha256(data).digest()).digest()


def compact_to_target(nbits: int) -> tuple[int, bool, bool]:
    """Convert compact nBits to (target, negative, overflow).

    Mirrors arith_uint256::SetCompact(fNegative, fOverflow): the sign bit
    (0x00800000 of the mantissa) marks negative; targets > 256 bits overflow.
    """
    size = nbits >> 24
    mantissa = nbits & 0x007FFFFF
    negative = bool(nbits & 0x00800000)
    if size <= 3:
        target = mantissa >> (8 * (3 - size))
    else:
        target = mantissa << (8 * (size - 3))
    overflow = target.bit_length() > 256
    return target, negative, overflow


def check_pow(header80: bytes, nbits: int) -> tuple[bool, str]:
    """Validate 80-byte header hash against compact target. Returns (ok, reason)."""
    if len(header80) != 80:
        return False, "bad-header-length"
    target, negative, overflow = compact_to_target(nbits)
    if negative or target == 0:
        return False, "negative-or-zero-target"
    if overflow:
        return False, "target-overflow"
    digest = sha256d(header80)
    value = int.from_bytes(digest, "little")  # internal order, like uint256 GetHash
    if value > target:
        return False, "hash-above-target"
    return True, "ok"


def serialize_header(version: int, prev_hash_le: bytes, merkle_root_le: bytes,
                     ntime: int, nbits: int, nonce: int) -> bytes:
    """80-byte header serialization. Hashes must be passed in little-endian
    (internal) order, i.e. reversed relative to block-explorer display order."""
    assert len(prev_hash_le) == 32 and len(merkle_root_le) == 32
    return (struct.pack("<i", version) + prev_hash_le + merkle_root_le
            + struct.pack("<III", ntime, nbits, nonce))


def display(hex_be: str) -> bytes:
    """Block-explorer (big-endian display) hex -> internal little-endian bytes."""
    return bytes.fromhex(hex_be)[::-1]
