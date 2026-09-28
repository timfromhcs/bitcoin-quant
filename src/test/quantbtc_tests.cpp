// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// QuantBTC chain-identity tests (§25-§29, §259): the new chain must carry
// no active Bitcoin Mainnet identity. Genesis replacement is covered by the
// genesis phase; these tests pin the network/address/seed isolation.
#include <chainparams.h>
#include <chainparamsbase.h>
#include <common/args.h>
#include <kernel/chainparams.h>
#include <key.h>
#include <key_io.h>
#include <pubkey.h>
#include <script/script.h>
#include <util/chaintype.h>

#include <boost/test/unit_test.hpp>
#include <test/util/setup_common.h>

// use BasicTestingSetup here for the data directory configuration, setup, and cleanup
BOOST_FIXTURE_TEST_SUITE(quantbtc_tests, BasicTestingSetup)

static const CChainParams& Main()
{
    static const auto params{CreateChainParams(ArgsManager{}, ChainType::MAIN)};
    return *params;
}

BOOST_AUTO_TEST_CASE(chain_id)
{
    BOOST_CHECK_EQUAL(Main().ChainID(), "QBTC-1");
}

BOOST_AUTO_TEST_CASE(network_magic)
{
    const auto& magic{Main().MessageStart()};
    BOOST_CHECK_EQUAL(magic[0], 0x3c);
    BOOST_CHECK_EQUAL(magic[1], 0x59);
    BOOST_CHECK_EQUAL(magic[2], 0x66);
    BOOST_CHECK_EQUAL(magic[3], 0x52);
    // Cross-network: QuantBTC magic resolves to MAIN ...
    BOOST_CHECK(GetNetworkForMagic(magic) == ChainType::MAIN);
    // ... while every Bitcoin magic is rejected (no silent cross-network).
    // (All four networks now carry QuantBTC magics; none resolve to Bitcoin's.)
    const std::vector<MessageStartChars> bitcoin_magics{
        {0xf9, 0xbe, 0xb4, 0xd9}, // Bitcoin main
        {0x0b, 0x11, 0x09, 0x07}, // Bitcoin testnet3
        {0x1c, 0x16, 0x3f, 0x28}, // Bitcoin testnet4
        {0xfa, 0xbf, 0xb5, 0xda}, // Bitcoin regtest
    };
    for (const auto& m : bitcoin_magics) {
        BOOST_CHECK(!GetNetworkForMagic(m).has_value());
    }
    // Every configured QuantBTC network resolves to its own chain type.
    const auto regtest{CreateChainParams(ArgsManager{}, ChainType::REGTEST)};
    BOOST_CHECK(GetNetworkForMagic(regtest->MessageStart()) == ChainType::REGTEST);
    const auto testnet{CreateChainParams(ArgsManager{}, ChainType::TESTNET)};
    BOOST_CHECK(GetNetworkForMagic(testnet->MessageStart()) == ChainType::TESTNET);
    const auto testnet4{CreateChainParams(ArgsManager{}, ChainType::TESTNET4)};
    BOOST_CHECK(GetNetworkForMagic(testnet4->MessageStart()) == ChainType::TESTNET4);
}

BOOST_AUTO_TEST_CASE(network_ports)
{
    BOOST_CHECK_EQUAL(Main().GetDefaultPort(), 8444);
    const auto base{CreateBaseChainParams(ArgsManager{}, ChainType::MAIN)};
    BOOST_CHECK_EQUAL(base->RPCPort(), 8442);
    const auto regtest{CreateChainParams(ArgsManager{}, ChainType::REGTEST)};
    BOOST_CHECK_EQUAL(regtest->GetDefaultPort(), 28445);
}

BOOST_AUTO_TEST_CASE(address_prefixes)
{
    BOOST_CHECK_EQUAL(Main().Bech32HRP(), "qb");
    BOOST_CHECK_EQUAL(Main().SilentPaymentsHRP(), "qsp");
    BOOST_CHECK_EQUAL(Main().Base58Prefix(CChainParams::PUBKEY_ADDRESS).at(0), 58);
    BOOST_CHECK_EQUAL(Main().Base58Prefix(CChainParams::SCRIPT_ADDRESS).at(0), 120);
    // First-character claims (§29): a P2PKH address must lead with 'Q'.
    // EncodeDestination uses the globally selected params.
    SelectParams(ChainType::MAIN);
    CKey key;
    key.MakeNewKey(true);
    const std::string p2pkh{EncodeDestination(PKHash(key.GetPubKey()))};
    BOOST_CHECK(!p2pkh.empty() && p2pkh[0] == 'Q');
    const std::string p2sh{EncodeDestination(ScriptHash(CScript() << OP_TRUE))};
    BOOST_CHECK(!p2sh.empty() && p2sh[0] == 'q');
}

BOOST_AUTO_TEST_CASE(no_bitcoin_seeds)
{
    BOOST_CHECK(Main().DNSSeeds().empty());
    BOOST_CHECK(Main().FixedSeeds().empty());
}

BOOST_AUTO_TEST_CASE(no_bitcoin_trust_anchors)
{
    const auto& consensus{Main().GetConsensus()};
    BOOST_CHECK(consensus.nMinimumChainWork.IsNull());
    BOOST_CHECK(consensus.defaultAssumeValid.IsNull());
    BOOST_CHECK(!Main().AssumeutxoForHeight(840'000).has_value());
    BOOST_CHECK_EQUAL(Main().TxData().nTime, 0);
    BOOST_CHECK_EQUAL(Main().TxData().tx_count, 0);
}

BOOST_AUTO_TEST_SUITE_END()
