// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <nft/record.h>
#include <test/nft_vectors.h>
#include <uint256.h>
#include <util/strencodings.h>

#include <boost/test/unit_test.hpp>

#include <cstring>
#include <vector>

static uint256 RawHex256(const std::string& hex)
{
    std::vector<unsigned char> b{ParseHex(hex)};
    BOOST_REQUIRE_EQUAL(b.size(), 32U);
    uint256 h;
    std::memcpy(h.begin(), b.data(), 32);
    return h;
}

BOOST_AUTO_TEST_SUITE(nft_tests)

//! Differential: C++ codec == Python reference byte-identical; nft_id matches.
BOOST_AUTO_TEST_CASE(mint_differential)
{
    BOOST_REQUIRE(!NFT_VECTORS.empty());
    for (const NftVector& v : NFT_VECTORS) {
        nft::MintRecord rec;
        rec.collection_id = RawHex256(v.cid_hex);
        rec.serial = v.serial;
        rec.content_root = RawHex256(v.root_hex);
        rec.metadata_hash = RawHex256(v.meta_hex);
        rec.owner_script = ParseHex(v.script_hex);
        rec.minted_epoch = v.epoch;
        BOOST_CHECK(nft::EncodeMint(rec) == ParseHex(v.blob_hex));
        nft::MintRecord dec;
        BOOST_REQUIRE(nft::DecodeMint(ParseHex(v.blob_hex).data(),
                                      ParseHex(v.blob_hex).size(), dec) ==
                      nft::MintError::Ok);
        BOOST_CHECK(dec.collection_id == rec.collection_id);
        BOOST_CHECK_EQUAL(dec.serial, rec.serial);
        BOOST_CHECK(dec.content_root == rec.content_root);
        BOOST_CHECK(dec.metadata_hash == rec.metadata_hash);
        BOOST_CHECK(dec.owner_script == rec.owner_script);
        BOOST_CHECK_EQUAL(dec.minted_epoch, rec.minted_epoch);
        BOOST_CHECK(nft::NftId(rec.collection_id, rec.serial) == RawHex256(v.id_hex));
    }
}

//! nft_id binds exactly (collection, serial): determinism + separation.
BOOST_AUTO_TEST_CASE(nft_id_binding)
{
    const uint256 cid = RawHex256(NFT_VECTORS.front().cid_hex);
    BOOST_CHECK(nft::NftId(cid, 7) == nft::NftId(cid, 7));
    BOOST_CHECK(!(nft::NftId(cid, 7) == nft::NftId(cid, 8)));
    uint256 other = cid;
    other.begin()[0] ^= 1;
    BOOST_CHECK(!(nft::NftId(cid, 7) == nft::NftId(other, 7)));
}

//! Strict rejects: truncation, version bump, trailing byte.
BOOST_AUTO_TEST_CASE(mint_strict)
{
    const std::vector<unsigned char> good{ParseHex(NFT_VECTORS.front().blob_hex)};
    nft::MintRecord dec;
    BOOST_CHECK(nft::DecodeMint(good.data(), 10, dec) == nft::MintError::TooShort);
    BOOST_CHECK(nft::DecodeMint(nullptr, 0, dec) == nft::MintError::TooShort);
    std::vector<unsigned char> badver = good;
    badver[0] = 2;
    BOOST_CHECK(nft::DecodeMint(badver.data(), badver.size(), dec) ==
                nft::MintError::BadVersion);
    std::vector<unsigned char> trail = good;
    trail.push_back(0);
    BOOST_CHECK(nft::DecodeMint(trail.data(), trail.size(), dec) ==
                nft::MintError::BadScriptSize);
}

BOOST_AUTO_TEST_SUITE_END()
