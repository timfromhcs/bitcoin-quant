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
from test_framework.util import assert_equal, assert_raises_rpc_error

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

        # 5. Full spend lifecycle with a fresh custodial key (REAL signatures).
        created = node.createpqcaddress("ml-dsa-65")
        assert_equal(created["algorithm"], "ml-dsa-65")
        spend_addr = created["address"]
        assert spend_addr.startswith("qbrt1p")
        cust_desc = created["descriptor"]
        cust_pub = created["pubkey"]
        cust_keyid = hashlib.sha256(bytes.fromhex(cust_pub)).hexdigest()
        cust_spk = "51" + "23" + "01" + "0100" + cust_keyid
        fund2 = spend.sendtoaddress(spend_addr, 2.0)
        self.generate(node, 1)
        # Locate the funding output.
        fund_tx = spend.gettransaction(fund2)["details"]
        vout = next(o["vout"] for o in fund_tx if o["address"] == spend_addr)
        # Build an unsigned spend back to the wallet (fee 0.001).
        change = spend.getnewaddress()
        raw = spend.createrawtransaction(
            [{"txid": fund2, "vout": vout}],
            {change: 1.999},
        )
        signed = node.signpqcwithkey(raw, [cust_desc], [{
            "txid": fund2, "vout": vout,
            "scriptPubKey": cust_spk, "amount": 2.0,
        }])
        assert_equal(signed["complete"], True)
        assert "errors" not in signed
        spend_txid = node.sendrawtransaction(signed["hex"])
        self.generate(node, 1)
        got = spend.gettransaction(spend_txid)
        assert_equal(got["confirmations"] >= 1, True)
        # Negative: unknown key cannot sign (fail closed, per-input error).
        other = node.createpqcaddress("ml-dsa-65")
        bad = node.signpqcwithkey(raw, [other["descriptor"]], [{
            "txid": fund2, "vout": vout,
            "scriptPubKey": cust_spk, "amount": 2.0,
        }])
        assert_equal(bad["complete"], False)
        assert_equal(len(bad["errors"]), 1)
        # Negative: missing amount throws fail-closed (BIP143 sighash needs value;
        # a request-shape error, not a signing failure).
        assert_raises_rpc_error(-8, "lacks required \"amount\"", node.signpqcwithkey, raw, [cust_desc], [{
            "txid": fund2, "vout": vout,
            "scriptPubKey": cust_spk,
        }])


if __name__ == "__main__":
    WalletP2PQTest(__file__).main()
