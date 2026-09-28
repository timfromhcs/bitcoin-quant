"""PoW + genesis reference tests (§37, §52-negativ, §150).

- Bitcoin mainnet genesis as KNOWN-ANSWER vector: proves contrib/quantbtc/ref/pow.py
  reproduces real consensus data (txid, merkle root, block hash, target).
- Compact edge cases: negative / zero / overflow targets must FAIL closed.
- Generator determinism: two independent runs must agree (§31.1-2, §204).
- Cross-network rejection: draft QuantBTC genesis must differ from every
  Bitcoin genesis hash (§261).
Stdlib only. Exit 0 = PASS, non-zero = FAIL (no fake success codes, §16).
"""
import io
import json
import sys
import unittest
from contextlib import redirect_stdout
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent / "ref"))
sys.path.insert(0, str(HERE))
from pow import sha256d, compact_to_target, check_pow, serialize_header, display  # noqa: E402
import generate_genesis as gen  # noqa: E402

TIMES_MSG = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks"
GENESIS_PK = ("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb"
              "649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f")
BITCOIN_GENESIS = {
    "txid": "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b",
    "merkle": "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b",
    "block": "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f",
    "time": 1231006505, "bits": 0x1D00FFFF, "nonce": 2083236893, "version": 1,
}
BITCOIN_GENESIS_HASHES = {  # all networks, must never equal a QuantBTC genesis
    "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f",  # main
    "000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943",  # test3
    "00000000da84f2bafbbc53dee25a72ae507ff4914b867c565be350b0da8bf043",  # test4
    "00000008819873e925422c1ff0f99f7cc9bbb232af63a077a480a3633bee1ef6",  # signet
    "0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206",  # regtest
}


def bitcoin_genesis_coinbase() -> bytes:
    scriptsig = (bytes.fromhex("04ffff001d") + b"\x01\x04"
                 + bytes([len(TIMES_MSG)]) + TIMES_MSG.encode("ascii"))
    assert len(scriptsig) == 77
    txin = (b"\x00" * 32 + b"\xff\xff\xff\xff" + bytes([77]) + scriptsig
            + b"\xff\xff\xff\xff")
    txout = (b"\x00\xf2\x05\x2a\x01\x00\x00\x00" + bytes([65 + 2])
             + bytes.fromhex("41" + GENESIS_PK + "ac"))
    return b"\x01\x00\x00\x00" + b"\x01" + txin + b"\x01" + txout + b"\x00\x00\x00\x00"


class TestCompact(unittest.TestCase):
    def test_genesis_target(self):
        target, neg, ovf = compact_to_target(0x1D00FFFF)
        self.assertFalse(neg)
        self.assertFalse(ovf)
        self.assertEqual(target, 0x00FFFF << (8 * (0x1D - 3)))

    def test_negative_rejected(self):
        _, neg, _ = compact_to_target(0x1D008000 | 0x00800000)
        self.assertTrue(neg)

    def test_zero_target_rejected(self):
        ok, reason = check_pow(b"\x00" * 80, 0x01000000)
        self.assertFalse(ok)

    def test_overflow_rejected(self):
        target, _, ovf = compact_to_target(0xFF00FFFF)
        self.assertTrue(ovf)
        ok, reason = check_pow(b"\x00" * 80, 0xFF00FFFF)
        self.assertFalse(ok)
        self.assertEqual(reason, "target-overflow")

    def test_bad_header_length(self):
        ok, reason = check_pow(b"\x00" * 79, 0x1D00FFFF)
        self.assertFalse(ok)


class TestBitcoinGenesisVector(unittest.TestCase):
    def test_coinbase_txid(self):
        txid_le = sha256d(bitcoin_genesis_coinbase())
        self.assertEqual(txid_le[::-1].hex(), BITCOIN_GENESIS["txid"])

    def test_block_hash_and_pow(self):
        g = BITCOIN_GENESIS
        header = serialize_header(g["version"], b"\x00" * 32,
                                  display(g["merkle"]), g["time"], g["bits"], g["nonce"])
        self.assertEqual(len(header), 80)
        self.assertEqual(sha256d(header)[::-1].hex(), g["block"])
        ok, reason = check_pow(header, g["bits"])
        self.assertTrue(ok, reason)

    def test_mutated_header_fails(self):
        g = BITCOIN_GENESIS
        header = bytearray(serialize_header(g["version"], b"\x00" * 32,
                                            display(g["merkle"]), g["time"],
                                            g["bits"], g["nonce"]))
        header[79] ^= 0x01  # flip one nonce bit
        ok, _ = check_pow(bytes(header), g["bits"])
        self.assertFalse(ok)


class TestGenerator(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with redirect_stdout(io.StringIO()):
            rc = gen.main()
        assert rc == 0, "generator failed"
        cls.first_hash = json.loads(
            (HERE / "generated" / "genesis-manifest.json").read_text())["genesis_hash"]

    def test_deterministic_twice(self):
        with redirect_stdout(io.StringIO()):
            rc = gen.main()
        self.assertEqual(rc, 0)
        manifest = json.loads((HERE / "generated" / "genesis-manifest.json").read_text())
        self.assertEqual(manifest["genesis_hash"], self.first_hash)

    def test_manifest_pow_valid(self):
        manifest = json.loads((HERE / "generated" / "genesis-manifest.json").read_text())
        header = bytes.fromhex((HERE / "generated" / "genesis-block.hex").read_text().strip())
        ok, reason = check_pow(header, int(manifest["nbits"], 16))
        self.assertTrue(ok, reason)
        self.assertEqual(sha256d(header)[::-1].hex(), manifest["genesis_hash"])

    def test_cross_network_rejection(self):
        manifest = json.loads((HERE / "generated" / "genesis-manifest.json").read_text())
        self.assertNotIn(manifest["genesis_hash"], BITCOIN_GENESIS_HASHES)
        self.assertNotEqual(manifest["tx_hash"], BITCOIN_GENESIS["txid"])


if __name__ == "__main__":
    unittest.main(verbosity=2)
