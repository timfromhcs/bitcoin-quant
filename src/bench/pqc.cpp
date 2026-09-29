// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bench/bench.h>
#include <pqc/backend.h>
#include <pqc/backend_oqs.h>
#include <pqc/backend_test.h>

#include <cassert>
#include <vector>

using namespace pqc;

static void PQCTestKeygen(benchmark::Bench& bench)
{
    RegisterDefaultPQCBackends();
    auto be{GetPQCBackend("test")};
    assert(be);
    bench.run([&] {
        auto kp{be->Generate(PQCAlgorithm::TEST_XOR)};
        assert(kp.has_value());
        ankerl::nanobench::doNotOptimizeAway(kp->pubkey.data());
    });
}

#ifdef HAVE_LIBOQS
static void PQCMldsa65Keygen(benchmark::Bench& bench)
{
    RegisterDefaultPQCBackends();
    auto be{GetPQCBackend("liboqs")};
    assert(be);
    bench.run([&] {
        auto kp{be->Generate(PQCAlgorithm::ML_DSA_65)};
        assert(kp.has_value());
        ankerl::nanobench::doNotOptimizeAway(kp->pubkey.data());
    });
}

static void PQCMldsa65Sign(benchmark::Bench& bench)
{
    RegisterDefaultPQCBackends();
    auto be{GetPQCBackend("liboqs")};
    assert(be);
    auto kp{be->Generate(PQCAlgorithm::ML_DSA_65)};
    assert(kp.has_value());
    const std::vector<unsigned char> msg(64, 0x55);
    bench.run([&] {
        auto sig{be->Sign(*kp, msg)};
        assert(sig.has_value());
        ankerl::nanobench::doNotOptimizeAway(sig->bytes.data());
    });
}

static void PQCMldsa65Verify(benchmark::Bench& bench)
{
    RegisterDefaultPQCBackends();
    auto be{GetPQCBackend("liboqs")};
    assert(be);
    auto kp{be->Generate(PQCAlgorithm::ML_DSA_65)};
    assert(kp.has_value());
    const std::vector<unsigned char> msg(64, 0x55);
    auto sig{be->Sign(*kp, msg)};
    assert(sig.has_value());
    bench.run([&] {
        assert(be->Verify(kp->pubkey, msg, *sig) == PQCError::OK);
    });
}
#endif

BENCHMARK(PQCTestKeygen);
#ifdef HAVE_LIBOQS
BENCHMARK(PQCMldsa65Keygen);
BENCHMARK(PQCMldsa65Sign);
BENCHMARK(PQCMldsa65Verify);
#endif
