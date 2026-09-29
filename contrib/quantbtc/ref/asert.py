"""aserti3-2d integer reference (§39, §114 differential-testing reference).

Direct port of the normative pseudocode (2020-11-15 ASERT spec, BCH/Nexa
upgrade specifications). Pure integers, no floats. Used to generate test
vectors for the C++ consensus implementation and to cross-check it.

QuantBTC adaptation (D-014, documented deviation): the BCH precondition
forbids anchor height 0 (no parent exists). A new chain has no history to
anchor to, so the anchor is genesis (height 0) with anchor_parent_time :=
genesis time itself. For a steady chain this yields exponent 0 exactly;
using T0-600 instead would inject a permanent +600s bias.
"""
import math

IDEAL_BLOCK_TIME = 600
HALFLIFE = 172_800
RADIX = 1 << 16
MAX_BITS = 0x1D00FFFF


def trunc_div(a: int, b: int) -> int:
    """Truncating division (C++ semantics); Python // floors."""
    assert b > 0
    q = abs(a) // abs(b)
    return -q if (a < 0) != (b < 0) else q


def bits_to_target(nbits: int) -> int:
    size = nbits >> 24
    mant = nbits & 0xFFFFFF
    if size <= 3:
        return mant >> (8 * (3 - size))
    return mant << (8 * (size - 3))


def target_to_bits(target: int) -> int:
    assert target >= 0
    if target == 0:
        return 0
    size = (target.bit_length() + 7) // 8
    if size <= 3:
        compact = (target & 0xFFFFFF) << (8 * (3 - size))
    else:
        compact = target >> (8 * (size - 3))
    if compact & 0x800000:
        compact >>= 8
        size += 1
    assert size < 256
    return (size << 24) | (compact & 0x7FFFFF)


MAX_TARGET = bits_to_target(MAX_BITS)


def next_target_aserti3_2d(anchor_height: int, anchor_parent_time: int,
                           anchor_bits: int, cur_height: int,
                           cur_time: int) -> int:
    """Returns next-block nBits. Mirrors the spec pseudocode line by line."""
    anchor_target = bits_to_target(anchor_bits)
    assert 0 < anchor_target <= MAX_TARGET
    time_delta = cur_time - anchor_parent_time
    height_delta = cur_height - anchor_height
    exponent = trunc_div((time_delta - IDEAL_BLOCK_TIME * (height_delta + 1)) * RADIX,
                         HALFLIFE)
    num_shifts = exponent >> 16  # arithmetic (floor) shift
    exponent = exponent - num_shifts * RADIX
    factor = ((195_766_423_245_049 * exponent
               + 971_821_376 * exponent ** 2
               + 5_127 * exponent ** 3
               + 2 ** 47) >> 48) + RADIX
    if num_shifts >= 512:  # sound early-outs (proof in DECISIONS D-015)
        return MAX_BITS
    if num_shifts <= -512:
        return target_to_bits(1)
    next_target = anchor_target * factor
    if num_shifts < 0:
        next_target >>= -num_shifts
    else:
        next_target <<= num_shifts
    next_target >>= 16
    if next_target == 0:
        return target_to_bits(1)
    if next_target > MAX_TARGET:
        return MAX_BITS
    return target_to_bits(next_target)


def next_target_float(anchor_bits: int, time_delta: int, height_delta: int) -> int:
    """Closed-form float version (validation oracle, NOT consensus)."""
    anchor_target = bits_to_target(anchor_bits)
    t = anchor_target * 2.0 ** ((time_delta - IDEAL_BLOCK_TIME * (height_delta + 1)) / HALFLIFE)
    if t > MAX_TARGET:
        return MAX_BITS
    if t < 1:
        return target_to_bits(1)
    return target_to_bits(int(t))


def _selftest():
    # compact round-trips incl. edge cases
    assert bits_to_target(0x1D00FFFF) == 0xFFFF << 208
    assert target_to_bits(bits_to_target(0x1D00FFFF)) == 0x1D00FFFF
    assert target_to_bits(bits_to_target(0x1B0404CB)) == 0x1B0404CB
    # steady chain from genesis anchor keeps difficulty-1 (exact)
    T0 = 1758931200
    for h in range(0, 5000, 611):
        assert next_target_aserti3_2d(0, T0, 0x1D00FFFF, h, T0 + (h + 1) * 600) == 0x1D00FFFF, h
    # faster blocks -> harder (smaller target), slower -> easier
    # (anchor below max so easing has headroom; at max, easing clamps)
    fast = next_target_aserti3_2d(0, T0, 0x1C00FFFF, 100, T0 + 101 * 300)
    slow = next_target_aserti3_2d(0, T0, 0x1C00FFFF, 100, T0 + 101 * 1200)
    mid = bits_to_target(0x1C00FFFF)
    assert bits_to_target(fast) < mid < bits_to_target(slow)
    # clamps
    assert next_target_aserti3_2d(0, T0, 0x1D00FFFF, 0, T0 + 10 * 172800) == MAX_BITS
    # float agreement (compact-exact or adjacent)
    import random
    rng = random.Random(0xA5E37)
    for _ in range(3000):
        h = rng.randrange(0, 200000)
        dt = rng.randrange(-100000, 30000000)
        a = next_target_aserti3_2d(0, T0, 0x1D00FFFF, h, T0 + dt)
        b = next_target_float(0x1D00FFFF, dt - 0, h)
        ta, tb = bits_to_target(a), bits_to_target(b)
        assert ta and tb
        ratio = ta / tb if ta > tb else tb / ta
        assert ratio < 1.02, (h, dt, hex(a), hex(b))
    print("asert self-test: PASS")


if __name__ == "__main__":
    _selftest()
