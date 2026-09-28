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
#include <arith_uint256.h>
#include <key.h>
#include <key_io.h>
#include <pow.h>
#include <pubkey.h>
#include <script/script.h>
#include <util/chaintype.h>

#include <set>

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

BOOST_AUTO_TEST_CASE(genesis_block)
{
    // Frozen QuantBTC genesis (protocol v1, see contrib/quantbtc/genesis/
    // generated/genesis-manifest.json). Must match the independent Python
    // generator bit-for-bit.
    const CBlock& genesis{Main().GenesisBlock()};
    BOOST_CHECK_EQUAL(genesis.GetHash().GetHex(),
        "000000002f24a967129873ad204d29f947f0452710c72c9aacf45fcf2d8f2881");
    BOOST_CHECK_EQUAL(genesis.hashMerkleRoot.GetHex(),
        "bab4d3bab87e3ca9493b99d64f7a4db66a9b064ec7128225da032e9bdef2f9c1");
    BOOST_CHECK_EQUAL(genesis.nTime, 1758931200);
    BOOST_CHECK_EQUAL(genesis.nBits, 0x1d00ffffu);
    BOOST_CHECK(CheckProofOfWork(genesis.GetHash(), genesis.nBits, Main().GetConsensus()));
    // Cross-network rejection: the QuantBTC genesis is none of Bitcoin's.
    static const std::set<std::string> bitcoin_geneses{
        "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f",
        "000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943",
        "00000000da84f2bafbbc53dee25a72ae507ff4914b867c565be350b0da8bf043",
        "00000008819873e925422c1ff0f99f7cc9bbb232af63a077a480a3633bee1ef6",
        "0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206",
    };
    BOOST_CHECK(!bitcoin_geneses.contains(genesis.GetHash().GetHex()));
}

BOOST_AUTO_TEST_CASE(pow_negative)
{
    // §37 proof suite anchored on the frozen genesis (§114 differential anchor
    // to contrib/quantbtc/ref/pow.py, which covers the same cases).
    const CBlock& genesis{Main().GenesisBlock()};
    const auto& consensus{Main().GetConsensus()};
    // Compact decoding: 0x1d00ffff must be exactly the difficulty-1 target.
    arith_uint256 target;
    bool neg = false, over = false;
    target.SetCompact(genesis.nBits, &neg, &over);
    BOOST_CHECK(!neg && !over);
    BOOST_CHECK_EQUAL(target.GetHex(),
        "00000000ffff0000000000000000000000000000000000000000000000000000");
    // Mutated nonce must fail.
    CBlock mutated{genesis};
    mutated.nNonce = genesis.nNonce ^ 1u;
    BOOST_CHECK(!CheckProofOfWork(mutated.GetHash(), mutated.nBits, consensus));
    // Harder target (256x) must fail for the frozen hash.
    BOOST_CHECK(!CheckProofOfWork(genesis.GetHash(), 0x1c00ffffu, consensus));
    // Overflow / negative compacts must fail closed.
    BOOST_CHECK(!CheckProofOfWork(genesis.GetHash(), 0xff00ffffu, consensus));
    BOOST_CHECK(!CheckProofOfWork(genesis.GetHash(), 0x1d80ffffu, consensus));
}

BOOST_AUTO_TEST_SUITE_END()
