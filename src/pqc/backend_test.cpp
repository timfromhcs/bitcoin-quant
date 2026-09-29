// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pqc/backend_test.h>

#include <crypto/sha256.h>

#include <atomic>
#include <cstring>

namespace pqc {
namespace {
// Deterministic key stream: counter restarts at 0 per process, so sequences
// are reproducible across runs. Distinct within a run by construction.
std::atomic<uint64_t>& KeyCounter()
{
    static std::atomic<uint64_t> counter{0};
    return counter;
}
void HashDomain(const char* tag, PQCAlgorithm algo, uint64_t ctr, std::span<const unsigned char> extra, unsigned char out32[32])
{
    CSHA256 hasher;
    hasher.Write((const unsigned char*)tag, strlen(tag));
    const auto id{static_cast<uint16_t>(algo)};
    unsigned char hdr[10];
    hdr[0] = id & 0xff;
    hdr[1] = (id >> 8) & 0xff;
    for (int i = 0; i < 8; ++i) hdr[2 + i] = (ctr >> (8 * i)) & 0xff;
    hasher.Write(hdr, 10).Write(extra.data(), extra.size()).Finalize(out32);
}
} // namespace

util::Expected<PQCKeyPair, PQCError> TestBackend::Generate(PQCAlgorithm algo)
{
    if (algo != PQCAlgorithm::TEST_XOR) return util::Unexpected(PQCError::UNSUPPORTED_ALGORITHM);
    const uint64_t ctr{KeyCounter().fetch_add(1)};
    // Stateless test double: the signature is a deterministic function of the
    // PUBLIC key and message (a real scheme would bind the secret key; the
    // verifier only ever sees the public key, so this preserves the
    // generate/sign/verify contract shape for plumbing tests).
    PQCKeyPair kp;
    kp.algo = algo;
    kp.privkey.resize(32);
    kp.pubkey.resize(32);
    const std::span<const unsigned char> empty{};
    HashDomain("QuantBTC-PQC-TEST-v1|priv|", algo, ctr, empty, kp.privkey.data());
    HashDomain("QuantBTC-PQC-TEST-v1|pub|", algo, ctr, empty, kp.pubkey.data());
    return kp;
}

util::Expected<PQCSignature, PQCError> TestBackend::Sign(const PQCKeyPair& key, std::span<const unsigned char> msg)
{
    if (key.algo != PQCAlgorithm::TEST_XOR || !key.IsValid()) {
        return util::Unexpected(PQCError::WRONG_KEY);
    }
    PQCSignature sig;
    sig.algo = PQCAlgorithm::TEST_XOR;
    sig.bytes.resize(64);
    unsigned char h1[32], h2[32];
    HashDomain("QuantBTC-PQC-TEST-v1|sig1|", key.algo, 0, key.pubkey, h1);
    // Bind message: hash domain includes pubkey then message.
    CSHA256 m1;
    m1.Write((const unsigned char*)"QuantBTC-PQC-TEST-v1|sigmsg|", 29).Write(key.pubkey.data(), key.pubkey.size()).Write(msg.data(), msg.size()).Finalize(h2);
    memcpy(sig.bytes.data(), h1, 32);
    memcpy(sig.bytes.data() + 32, h2, 32);
    return sig;
}

PQCError TestBackend::Verify(std::span<const unsigned char> pubkey, std::span<const unsigned char> msg, const PQCSignature& sig)
{
    if (sig.algo != PQCAlgorithm::TEST_XOR) return PQCError::WRONG_KEY;
    if (sig.bytes.size() != 64) return PQCError::BAD_SIZE;
    if (pubkey.size() != 32) return PQCError::BAD_SIZE;
    unsigned char h1[32], h2[32];
    HashDomain("QuantBTC-PQC-TEST-v1|sig1|", PQCAlgorithm::TEST_XOR, 0, pubkey, h1);
    CSHA256 m1;
    m1.Write((const unsigned char*)"QuantBTC-PQC-TEST-v1|sigmsg|", 29).Write(pubkey.data(), pubkey.size()).Write(msg.data(), msg.size()).Finalize(h2);
    if (memcmp(sig.bytes.data(), h1, 32) != 0) return PQCError::VERIFY_FAILED;
    if (memcmp(sig.bytes.data() + 32, h2, 32) != 0) return PQCError::VERIFY_FAILED;
    return PQCError::OK;
}

} // namespace pqc
