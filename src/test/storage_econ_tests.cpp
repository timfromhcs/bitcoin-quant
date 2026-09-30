// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <storage/econ.h>
#include <test/storage_econ_vectors.h>
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

BOOST_AUTO_TEST_SUITE(storage_econ_tests)

//! Differential: C++ reward math == Python big-int oracle on every vector,
//! including 2^64 limb-stress and floor edges.
BOOST_AUTO_TEST_CASE(reward_differential)
{
    BOOST_REQUIRE(!STORAGE_REWARD_VECTORS.empty());
    for (const StorageRewardVector& v : STORAGE_REWARD_VECTORS) {
        int64_t amount = 0;
        const storage::RewardResult r =
            storage::CalcReward(v.capacity, v.price, v.verified, amount);
        // Vector codes: 0=OK 1=OVERFLOW 2=INVALID_INPUT (see header).
        storage::RewardResult want = storage::RewardResult::REWARD_INVALID_INPUT;
        if (v.cls == 0) want = storage::RewardResult::REWARD_OK;
        if (v.cls == 1) want = storage::RewardResult::REWARD_OVERFLOW;
        BOOST_CHECK(r == want);
        if (r == storage::RewardResult::REWARD_OK) BOOST_CHECK_EQUAL(amount, v.amount);
    }
}

//! Floor + exactness spot checks with hand-verified values.
BOOST_AUTO_TEST_CASE(reward_floor_exact)
{
    int64_t amount = -1;
    BOOST_CHECK(storage::CalcReward(1ULL << 30, 7, (1ULL << 30) - 1, amount) ==
                storage::RewardResult::REWARD_OK);
    BOOST_CHECK_EQUAL(amount, 6); // floor((2^30-1)*7/2^30) = 6
    BOOST_CHECK(storage::CalcReward(100, 1, 99, amount) == storage::RewardResult::REWARD_OK);
    BOOST_CHECK_EQUAL(amount, 0);
    BOOST_CHECK(storage::CalcReward((1ULL << 40), 1000000, (1ULL << 40), amount) ==
                storage::RewardResult::REWARD_OK);
    BOOST_CHECK_EQUAL(amount, (int64_t)((1000000ULL * (1ULL << 40)) >> 30));
    // Monotonicity over a sweep (no float anywhere in the path).
    int64_t prev = -1;
    for (uint64_t v = 0; v <= 3ULL << 30; v += 10000000ULL) {
        BOOST_REQUIRE(storage::CalcReward(3ULL << 30, 1000000000ULL, v, amount) ==
                      storage::RewardResult::REWARD_OK);
        BOOST_CHECK(amount >= prev);
        prev = amount;
    }
}

//! Invalid inputs fail closed, never wrap.
BOOST_AUTO_TEST_CASE(reward_invalid)
{
    int64_t amount = 0;
    BOOST_CHECK(storage::CalcReward(0, 1, 0, amount) == storage::RewardResult::REWARD_INVALID_INPUT);
    BOOST_CHECK(storage::CalcReward(100, 1, 101, amount) == storage::RewardResult::REWARD_INVALID_INPUT);
    BOOST_CHECK(storage::CalcReward(100, storage::MAX_STORAGE_PRICE + 1, 50, amount) ==
                storage::RewardResult::REWARD_INVALID_INPUT);
    BOOST_CHECK(storage::CalcReward(~0ULL, storage::MAX_STORAGE_PRICE, ~0ULL, amount) ==
                storage::RewardResult::REWARD_OVERFLOW); // max limbs, exact, no wrap
}

//! Record codec round-trip + strict rejects.
BOOST_AUTO_TEST_CASE(record_codec)
{
    BOOST_REQUIRE(!STORAGE_RECORD_VECTORS.empty());
    for (const StorageRecordVector& v : STORAGE_RECORD_VECTORS) {
        storage::ProviderRecord rec;
        rec.provider_id = RawHex256(v.pid_hex);
        rec.payout_script = ParseHex(v.script_hex);
        rec.capacity_bytes = v.capacity;
        rec.price_sat_per_gib_epoch = v.price;
        rec.status = static_cast<storage::ProviderStatus>(v.status);
        rec.registered_epoch = v.epoch;
        const std::vector<unsigned char> enc = storage::EncodeRecord(rec);
        BOOST_CHECK(enc == ParseHex(v.blob_hex)); // byte-identical to Python
        storage::ProviderRecord dec;
        BOOST_REQUIRE(storage::DecodeRecord(enc.data(), enc.size(), dec) ==
                      storage::RecordError::OK);
        BOOST_CHECK(dec.provider_id == rec.provider_id);
        BOOST_CHECK(dec.payout_script == rec.payout_script);
        BOOST_CHECK_EQUAL(dec.capacity_bytes, rec.capacity_bytes);
        BOOST_CHECK_EQUAL(dec.price_sat_per_gib_epoch, rec.price_sat_per_gib_epoch);
        BOOST_CHECK(dec.status == rec.status);
        BOOST_CHECK_EQUAL(dec.registered_epoch, rec.registered_epoch);
    }
    // Strict rejects.
    const std::vector<unsigned char> good{
        ParseHex(STORAGE_RECORD_VECTORS.front().blob_hex)};
    storage::ProviderRecord dec;
    BOOST_CHECK(storage::DecodeRecord(good.data(), 10, dec) == storage::RecordError::TOO_SHORT);
    std::vector<unsigned char> badver = good;
    badver[0] = 2;
    BOOST_CHECK(storage::DecodeRecord(badver.data(), badver.size(), dec) ==
                storage::RecordError::BAD_VERSION);
    std::vector<unsigned char> trail = good;
    trail.push_back(0);
    BOOST_CHECK(storage::DecodeRecord(trail.data(), trail.size(), dec) ==
                storage::RecordError::BAD_SCRIPT_SIZE);
    // Zero capacity / bad status / over-cap price rejected post-decode.
    std::vector<unsigned char> badcap = good; // capacity field offset: 1+32+2+slen
    const size_t slen = (size_t)good[33] | ((size_t)good[34] << 8);
    std::fill(badcap.begin() + 35 + slen, badcap.begin() + 43 + slen, 0);
    BOOST_CHECK(storage::DecodeRecord(badcap.data(), badcap.size(), dec) ==
                storage::RecordError::BAD_CAPACITY);
}

//! Status machine exhaustive.
BOOST_AUTO_TEST_CASE(status_machine)
{
    using S = storage::ProviderStatus;
    BOOST_CHECK(storage::TransitionOk(S::ACTIVE, S::SUSPENDED));
    BOOST_CHECK(storage::TransitionOk(S::ACTIVE, S::EXITED));
    BOOST_CHECK(!storage::TransitionOk(S::ACTIVE, S::ACTIVE));
    BOOST_CHECK(storage::TransitionOk(S::SUSPENDED, S::ACTIVE));
    BOOST_CHECK(storage::TransitionOk(S::SUSPENDED, S::EXITED));
    BOOST_CHECK(!storage::TransitionOk(S::SUSPENDED, S::SUSPENDED));
    BOOST_CHECK(!storage::TransitionOk(S::EXITED, S::ACTIVE));
    BOOST_CHECK(!storage::TransitionOk(S::EXITED, S::SUSPENDED));
    BOOST_CHECK(!storage::TransitionOk(S::EXITED, S::EXITED));
    BOOST_CHECK(!storage::TransitionOk(static_cast<S>(0), S::ACTIVE));
    BOOST_CHECK(!storage::TransitionOk(S::ACTIVE, static_cast<S>(9)));
}

BOOST_AUTO_TEST_SUITE_END()
