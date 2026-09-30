"""Storage economy reference (§87-§88, §116): provider record + rewards.

Pure math, stdlib-only, exact (Python big ints are the oracle for the C++
portable 128-bit limb implementation):
  amount = floor(verified_bytes * price_sat_per_GiB_epoch / 2**30)
  classes: OK | OVERFLOW (amount > MAX_MONEY) | INVALID_INPUT
Record v1 codec + structural validation + status machine included.
Out of scope (documented gaps): operator binding, registry uniqueness/
persistence, audit aggregation, consensus hook, payouts, full script checks.
"""
import struct

RECORD_VERSION = 1
MAX_SCRIPT_SIZE = 10000
MAX_PRICE = 21 * 10**14          # total-supply order bound (sat)
MAX_MONEY = 21000000 * 10**8     # like consensus MAX_MONEY (sat)
GIB = 2**30

STATUS_ACTIVE = 1
STATUS_SUSPENDED = 2
STATUS_EXITED = 3
STATUSES = (STATUS_ACTIVE, STATUS_SUSPENDED, STATUS_EXITED)
TRANSITIONS = {
    STATUS_ACTIVE: (STATUS_SUSPENDED, STATUS_EXITED),
    STATUS_SUSPENDED: (STATUS_ACTIVE, STATUS_EXITED),
    STATUS_EXITED: (),
}


def encode_record(provider_id: bytes, payout_script: bytes, capacity: int,
                  price: int, status: int, registered_epoch: int) -> bytes:
    assert len(provider_id) == 32
    assert 1 <= len(payout_script) <= MAX_SCRIPT_SIZE
    assert capacity > 0 and price >= 0 and status in STATUSES and registered_epoch >= 0
    out = bytes([RECORD_VERSION]) + provider_id
    out += struct.pack("<H", len(payout_script)) + payout_script
    out += struct.pack("<Q", capacity) + struct.pack("<Q", price)
    out += bytes([status]) + struct.pack("<Q", registered_epoch)
    return out


def decode_record(blob: bytes) -> tuple:
    """Returns (record_dict, error). Strict: exact length, version gate."""
    if len(blob) < 1 + 32 + 2 + 1 + 8 + 8 + 1 + 8:
        return None, "too-short"
    if blob[0] != RECORD_VERSION:
        return None, "bad-version"
    pos = 1
    provider_id = blob[pos:pos + 32]
    pos += 32
    (slen,) = struct.unpack("<H", blob[pos:pos + 2])
    pos += 2
    if slen == 0 or slen > MAX_SCRIPT_SIZE or len(blob) != pos + slen + 8 + 8 + 1 + 8:
        return None, "bad-script-size"
    script = blob[pos:pos + slen]
    pos += slen
    (capacity,) = struct.unpack("<Q", blob[pos:pos + 8])
    pos += 8
    (price,) = struct.unpack("<Q", blob[pos:pos + 8])
    pos += 8
    status = blob[pos]
    pos += 1
    (epoch,) = struct.unpack("<Q", blob[pos:pos + 8])
    rec = {"provider_id": provider_id, "payout_script": script,
           "capacity": capacity, "price": price, "status": status,
           "registered_epoch": epoch}
    err = validate_record(rec)
    if err != "ok":
        return None, err
    return rec, "ok"


def validate_record(rec: dict) -> str:
    if rec["status"] not in STATUSES:
        return "bad-status"
    if rec["capacity"] == 0:
        return "bad-capacity"
    if rec["price"] > MAX_PRICE:
        return "bad-price"
    if not (1 <= len(rec["payout_script"]) <= MAX_SCRIPT_SIZE):
        return "bad-script-size"
    if len(rec["provider_id"]) != 32:
        return "bad-provider-id"
    return "ok"


def transition_ok(old: int, new: int) -> bool:
    return new in TRANSITIONS.get(old, ())


def calc_reward(capacity: int, price: int, verified_bytes: int) -> tuple:
    """Returns (amount_sat|None, class). Exact floor division."""
    if capacity <= 0 or price < 0 or verified_bytes < 0:
        return None, "INVALID_INPUT"
    if price > MAX_PRICE or verified_bytes > capacity:
        return None, "INVALID_INPUT"
    amount = (verified_bytes * price) // GIB
    if amount > MAX_MONEY:
        return None, "OVERFLOW"
    return amount, "OK"


def _selftest():
    import random
    rng = random.Random(0xEC00)
    # codec round-trip incl. edges
    for slen in (1, 25, MAX_SCRIPT_SIZE):
        for status in STATUSES:
            rec, err = decode_record(encode_record(
                bytes(rng.randrange(256) for _ in range(32)),
                bytes(rng.randrange(256) for _ in range(slen)),
                rng.choice((1, 2**40, 2**64 - 1)),
                rng.choice((0, 1, 10**6, MAX_PRICE)),
                status, rng.choice((0, 2**64 - 1))))
            assert err == "ok", err
            assert rec["status"] == status
    # negatives
    good = encode_record(b"\x11" * 32, b"\x51", 1000, 100, STATUS_ACTIVE, 0)
    assert decode_record(b"")[1] == "too-short"
    assert decode_record(bytes([2]) + good[1:])[1] == "bad-version"
    assert decode_record(good + b"\x00")[1] == "bad-script-size"
    assert decode_record(good[:-9] + bytes([9]) + good[-8:])[1] == "bad-status"
    assert decode_record(good[:36] + b"\x00" * 8 + good[44:])[1] == "bad-capacity"
    # transitions exhaustive
    for a in STATUSES:
        for b in STATUSES:
            assert transition_ok(a, b) == (b in TRANSITIONS[a])
    assert not transition_ok(99, STATUS_ACTIVE)
    # rewards: exactness incl. near-2^64 limb stress + floor behavior
    cases = [
        (2**40, 10**6, 2**40),          # 1 TiB @ 1M sat/GiB
        (2**64 - 1, MAX_PRICE, 2**64 - 1),  # must be OVERFLOW, never wrap
        (2**64 - 1, 0, 2**64 - 1),      # zero price -> 0, still exact
        (100, 1, 99),                   # floor: 99/2^30 -> 0
        (GIB, 7, GIB - 1),              # floor: (2^30-1)*7/2^30 = 6
    ]
    for cap, price, ver in cases:
        amt, cls = calc_reward(cap, price, ver)
        want = (ver * price) // GIB
        if want > MAX_MONEY:
            assert cls == "OVERFLOW" and amt is None, (cap, price, ver)
        else:
            assert (amt, cls) == (want, "OK"), (cap, price, ver)
    assert calc_reward(GIB, 7, GIB - 1) == (6, "OK")
    assert calc_reward(0, 1, 0)[1] == "INVALID_INPUT"
    assert calc_reward(100, 1, 101)[1] == "INVALID_INPUT"   # verified > capacity
    assert calc_reward(100, MAX_PRICE + 1, 50)[1] == "INVALID_INPUT"
    assert calc_reward(100, 1, -1)[1] == "INVALID_INPUT"
    # monotonicity + no-float-error spot check
    prev = -1
    for v in range(0, 3 * GIB, 10**7):
        a, c = calc_reward(3 * GIB, 10**9, v)
        assert c == "OK" and a >= prev
        assert a == (v * 10**9) // GIB   # == float-free exactness
        prev = a
    print("storage econ self-test: PASS")


if __name__ == "__main__":
    _selftest()
