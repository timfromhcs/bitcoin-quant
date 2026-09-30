#!/usr/bin/env python3
# Copyright (c) 2026-present The QuantBTC developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test P2PQ watch-only wallet tracking (QuantBTC protocol v1).

Uses a fixed, clearly non-secret 1952-byte test vector as the ML-DSA-65
stand-in public key (watch-only needs no valid key, only a stable encoding).
Proves on real nodes: p2pq() descriptor import, bech32m address derivation,
funding, balance and UTXO tracking with exact consensus script bytes.
Spending (signing/custody) is a follow-up phase; solvability is asserted false.
"""
import hashlib

from test_framework.descriptors import descsum_create
from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import assert_equal

# Deterministic 1952-byte stand-in (NOT a real key; watch-only encoding test).
TEST_PUBKEY = bytes(i % 256 for i in range(1952))
TEST_KEYID = hashlib.sha256(TEST_PUBKEY).hexdigest()
# P2PQ scriptPubKey: OP_1 <0x23 <0x01 ver><0x0001 alg LE><32B keyid>>>
TEST_SPK = "51" + "23" + "01" + "0100" + TEST_KEYID


class WalletP2PQTest(BitcoinTestFramework):
    def set_test_params(self):
        self.num_nodes = 1
        self.setup_clean_chain = True

    def skip_test_if_missing_module(self):
        self.skip_if_no_wallet()

    def run_test(self):
        node = self.nodes[0]
        node.createwallet(wallet_name="wo", disable_private_keys=True)
        wallet = node.get_wallet_rpc("wo")
        spend = node.get_wallet_rpc(self.default_wallet_name)

        # 1. Import the p2pq() descriptor (watch-only).
        desc = descsum_create("p2pq(%s)" % TEST_PUBKEY.hex())
        res = wallet.importdescriptors([{
            "desc": desc,
            "active": False,
            "timestamp": "now",
        }])
        assert_equal(res[0]["success"], True)

        # 2. The node decodes the exact consensus script to a bech32m address.
        decoded = node.decodescript(TEST_SPK)
        assert_equal(decoded["type"], "witness_unknown")
        p2pq_addr = decoded["address"]
        assert p2pq_addr.startswith("qbrt1p")

        # 3. Fund it with real coins and confirm.
        self.generatetoaddress(node, 101, spend.getnewaddress())
        txid = spend.sendtoaddress(p2pq_addr, 1.0)
        self.generate(node, 1)

        # 4. The wallet tracks the P2PQ UTXO with exact script bytes.
        # ("spendable" is a deprecated always-true field; "solvable" is the
        # real signal — false here: no custody yet, watch-only.)
        unspent = wallet.listunspent(0, 9999, [p2pq_addr])
        assert_equal(len(unspent), 1)
        assert_equal(unspent[0]["txid"], txid)
        assert_equal(unspent[0]["scriptPubKey"], TEST_SPK)
        assert_equal(unspent[0]["solvable"], False)


if __name__ == "__main__":
    WalletP2PQTest(__file__).main()
