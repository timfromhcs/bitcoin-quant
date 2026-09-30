#!/usr/bin/env python3
# Copyright (c) 2026-present The QuantBTC developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test storage RPC surface (QuantBTC protocol v1, spec section 12).

Exercises commit -> challenge -> prove -> verify against real nodes with an
INDEPENDENT hashlib re-implementation of the spec (not the repo reference):
any divergence between C++ RPC and this test fails loudly.

Covers: multi-chunk/edge-size content, challenge determinism, proof
round-trip, fail-closed verify (valid=false + reason, never throws for
proof content), and strict RPC parameter validation.
"""
import hashlib
import random
import struct

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import assert_equal, assert_raises_rpc_error

CHUNK_SIZE = 65536


def dsha(b):
    return hashlib.sha256(hashlib.sha256(b).digest()).digest()


def merkle(leaves):
    assert leaves
    level = list(leaves)
    while len(level) > 1:
        if len(level) % 2 == 1:
            level.append(level[-1])
        level = [dsha(level[i] + level[i + 1]) for i in range(0, len(level), 2)]
    return level[0]


def commit(content):
    chunks = [content[i:i + CHUNK_SIZE] for i in range(0, len(content), CHUNK_SIZE)]
    hashes = [dsha(c) for c in chunks]
    return chunks, hashes, merkle(hashes)


def challenge(root, pid, epoch, n, k=16):
    seed = dsha(root + pid + struct.pack(">Q", epoch))
    return [int.from_bytes(dsha(seed + struct.pack(">I", i)), "big") % n for i in range(k)]


def prove_at(chunks, index):
    level = [dsha(c) for c in chunks]
    path, idx = [], index
    while len(level) > 1:
        if len(level) % 2 == 1:
            level.append(level[-1])
        path.append(level[idx ^ 1])
        level = [dsha(level[i] + level[i + 1]) for i in range(0, len(level), 2)]
        idx //= 2
    return chunks[index], path


class StorageRPCTest(BitcoinTestFramework):
    def set_test_params(self):
        self.num_nodes = 1
        self.setup_clean_chain = True

    def run_test(self):
        node = self.nodes[0]
        rng = random.Random(0x5702A6E)
        pid = dsha(b"rpc-storage-provider")

        for size in (1, CHUNK_SIZE - 1, CHUNK_SIZE, CHUNK_SIZE + 1, 3 * CHUNK_SIZE + 17):
            content = bytes(rng.randrange(256) for _ in range(size))
            chunks, hashes, root = commit(content)

            # commit: root, count, per-chunk hashes match independent impl.
            res = node.storagecommit(content.hex())
            assert_equal(res["root"], root.hex())
            assert_equal(res["chunks"], len(chunks))
            assert_equal(res["chunk_size"], CHUNK_SIZE)
            assert_equal(res["chunk_hashes"], [h.hex() for h in hashes])

            # challenge: indices match (default k=16 and explicit k).
            for epoch in (0, 7):
                want = challenge(root, pid, epoch, len(chunks))
                assert_equal(node.storagechallenge(root.hex(), pid.hex(), epoch, len(chunks))["indices"], want)
                want5 = challenge(root, pid, epoch, len(chunks), k=5)
                got5 = node.storagechallenge(root.hex(), pid.hex(), epoch, len(chunks), 5)["indices"]
                assert_equal(got5, want5)

            # prove every challenged index, verify each envelope.
            idxs = challenge(root, pid, 7, len(chunks))
            for idx in idxs:
                pr = node.storageprove(content.hex(), idx)
                chunk, path = prove_at(chunks, idx)
                assert_equal(pr["index"], idx)
                assert_equal(pr["chunk"], chunk.hex())
                assert_equal(pr["path"], [s.hex() for s in path])
                ver = node.storageverify(root.hex(), pr["envelope"])
                assert_equal(ver, {"valid": True, "reason": "ok"})

        # fail-closed verify negatives (valid=false, never throws).
        good = node.storageprove(content.hex(), 0)
        assert_equal(node.storageverify("00" * 32, good["envelope"])["reason"], "root-mismatch")
        assert_equal(node.storageverify("00" * 32, good["envelope"])["valid"], False)
        tampered = bytearray(bytes.fromhex(good["envelope"]))
        tampered[-1] ^= 1
        bad = node.storageverify(res["root"], tampered.hex())
        assert_equal(bad["valid"], False)
        assert bad["reason"] in ("root-mismatch", "malformed-proof", "bad-chunk-size")
        assert_equal(node.storageverify(res["root"], "ff")["reason"], "malformed-proof")
        assert_equal(node.storageverify(res["root"], "")["reason"], "malformed-proof")

        # strict parameter validation (RPC errors).
        assert_raises_rpc_error(-8, "Empty content", node.storagecommit, "")
        assert_raises_rpc_error(-8, "out of range", node.storageprove, content.hex(), len(chunks))
        assert_raises_rpc_error(-8, "out of range", node.storageprove, content.hex(), 10**9)
        assert_raises_rpc_error(-8, "nleaves", node.storagechallenge, res["root"], pid.hex(), 0, 0)
        assert_raises_rpc_error(-8, "samples", node.storagechallenge, res["root"], pid.hex(), 0, 1, 0)
        assert_raises_rpc_error(-8, "samples", node.storagechallenge, res["root"], pid.hex(), 0, 1, 1025)
        assert_raises_rpc_error(-8, "32-byte", node.storagechallenge, "ab" * 31, pid.hex(), 0, 1)
        assert_raises_rpc_error(-8, "Negative epoch", node.storagechallenge, res["root"], pid.hex(), -1, 1)
        assert_raises_rpc_error(-8, "32-byte", node.storageverify, "zz", good["envelope"])


if __name__ == "__main__":
    StorageRPCTest(__file__).main()
