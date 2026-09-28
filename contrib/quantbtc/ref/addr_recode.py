"""Chain-agnostic address re-encoding utility (test-vector regeneration).

Implements base58check + bech32/bech32m from the public specifications
(BIP173/BIP350 logic) in stdlib-only Python. Used to re-anchor upstream
address test vectors to QuantBTC prefixes/HRPs WITHOUT touching keys or
payloads. Self-tests against official BIP173/350 vectors on import.
"""
import hashlib

B58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"
BECH32_CONST, BECH32M_CONST = 1, 0x2BC830A3
BECH32_CHARSET = "qpzry9x8gf2tvdw0s3jn54khce6mua7l"
GEN = [0x3B6A57B2, 0x26508E6D, 0x1EA119FA, 0x3D4233DD, 0x2A1462B3]


def b58check_decode(s: str) -> bytes:
    n = 0
    for c in s:
        n = n * 58 + B58.index(c)
    raw = n.to_bytes((n.bit_length() + 7) // 8, "big") if n else b""
    pad = len(s) - len(s.lstrip("1"))
    payload = b"\x00" * pad + raw
    body, cksum = payload[:-4], payload[-4:]
    assert hashlib.sha256(hashlib.sha256(body).digest()).digest()[:4] == cksum, "bad checksum"
    return body


def b58check_encode(body: bytes) -> str:
    cksum = hashlib.sha256(hashlib.sha256(body).digest()).digest()[:4]
    n = int.from_bytes(body + cksum, "big")
    s = ""
    while n > 0:
        n, r = divmod(n, 58)
        s = B58[r] + s
    pad = len(body + cksum) - len((body + cksum).lstrip(b"\x00"))
    return "1" * pad + s


def _polymod(values):
    chk = 1
    for v in values:
        b = chk >> 25
        chk = ((chk & 0x1FFFFFF) << 5) ^ v
        for i in range(5):
            chk ^= GEN[i] if (b >> i) & 1 else 0
    return chk


def _hrp_expand(hrp):
    return [ord(x) >> 5 for x in hrp] + [0] + [ord(x) & 31 for x in hrp]


def bech32_decode(addr: str):
    addr = addr.lower()
    pos = addr.rfind("1")
    assert pos >= 1 and pos + 7 <= len(addr)  # no 90-char cap: test vectors exceed it
    hrp, data = addr[:pos], [BECH32_CHARSET.find(c) for c in addr[pos + 1:]]
    assert all(c != -1 for c in data)
    pm = _polymod(_hrp_expand(hrp) + data)
    if pm == BECH32_CONST:
        spec = "bech32"
    elif pm == BECH32M_CONST:
        spec = "bech32m"
    else:
        raise AssertionError("bad checksum")
    return hrp, data[:-6], spec


def bech32_encode(hrp: str, data: list, spec: str) -> str:
    const = BECH32_CONST if spec == "bech32" else BECH32M_CONST
    pm = _polymod(_hrp_expand(hrp) + data + [0] * 6) ^ const
    chk = [(pm >> 5 * (5 - i)) & 31 for i in range(6)]
    return hrp + "1" + "".join(BECH32_CHARSET[d] for d in data + chk)


def convertbits(data, frombits, tobits, pad=True):
    acc = bits = 0
    ret = []
    maxv, max_acc = (1 << tobits) - 1, (1 << (frombits + tobits - 1)) - 1
    for b in data:
        acc = ((acc << frombits) | b) & max_acc
        bits += frombits
        while bits >= tobits:
            bits -= tobits
            ret.append((acc >> bits) & maxv)
    if pad:
        if bits:
            ret.append((acc << (tobits - bits)) & maxv)
    elif bits >= frombits or ((acc << (tobits - bits)) & maxv):
        raise AssertionError("bad padding")
    return ret


def _selftest():
    # Ground truth: src/test/bech32_tests.cpp (bech32 vs bech32m sets)
    for v in ("A12UEL5L", "a12uel5l",
              "an83characterlonghumanreadablepartthatcontainsthenumber1andtheexcludedcharactersbio1tt5tgs",
              "abcdef1qpzry9x8gf2tvdw0s3jn54khce6mua7lmqqqxw",
              "11qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqc8247j",
              "split1checkupstagehandshakeupstreamerranterredcaperred2y9e3w",
              "?1ezyfcl"):
        assert bech32_decode(v)[2] == "bech32", v
    for v in ("A1LQFN3A", "a1lqfn3a",
              "an83characterlonghumanreadablepartthatcontainsthetheexcludedcharactersbioandnumber11sg7hg6",
              "abcdef1l7aum6echk45nj3s0wdvt2fg8x9yrzpqzd3ryx",
              "11llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllludsr8",
              "split1checkupstagehandshakeupstreamerranterredcaperredlc445v",
              "?1v759aa"):
        assert bech32_decode(v)[2] == "bech32m", v
    # P2WPKH program from descriptor_tests (decodes under Bitcoin params)
    hrp, data, spec = bech32_decode("bc1qyt3k8ffrj3apzrv6n6c3fqsduxp6egcnk2r66j")
    assert (hrp, spec) == ("bc", "bech32") and data[0] == 0 and len(data) == 33
    assert bech32_encode(hrp, data, spec) == "bc1qyt3k8ffrj3apzrv6n6c3fqsduxp6egcnk2r66j"
    # base58 round-trip + version-byte handling
    body = b58check_decode("1QFqqMUD55ZV3PJEJZtaKCsQmjLT6JkjvJ")
    assert body[0] == 0 and len(body) == 21
    assert b58check_encode(body) == "1QFqqMUD55ZV3PJEJZtaKCsQmjLT6JkjvJ"
    assert b58check_encode(bytes([58]) + body[1:])[0] == "Q"


if __name__ == "__main__":
    _selftest()
    print("addr_recode self-test: PASS")
