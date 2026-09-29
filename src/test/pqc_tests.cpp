// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// PQC abstraction tests (§46-§52): envelopes, registry, backends, P2PQ
// format, negative battery. Production claims require the liboqs backend;
// see PROOFS/pqc/ for what is (and is not yet) verified.
#include <pqc/algorithm.h>
#include <pqc/backend.h>
#include <pqc/backend_oqs.h>
#include <pqc/backend_test.h>
#include <pqc/encoding.h>
#include <pqc/error.h>
#include <pqc/key.h>

#include <crypto/sha256.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <test/data/mldsa65_kat.json.h>
#include <test/util/json.h>
#include <test/util/setup_common.h>
#include <uint256.h>
#include <util/strencodings.h>

#include <boost/test/unit_test.hpp>

using namespace pqc;

BOOST_FIXTURE_TEST_SUITE(pqc_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(algorithm_params)
{
    const auto mldsa{GetAlgorithmParams(PQCAlgorithm::ML_DSA_65)};
    BOOST_REQUIRE(mldsa.has_value());
    BOOST_CHECK_EQUAL(mldsa->pubkey_size, 1952U);
    BOOST_CHECK_EQUAL(mldsa->privkey_size, 4032U);
    BOOST_CHECK_EQUAL(mldsa->sig_size, 3309U);
    BOOST_CHECK(!GetAlgorithmParams(PQCAlgorithm::UNKNOWN).has_value());
    BOOST_CHECK(!GetAlgorithmParams(static_cast<PQCAlgorithm>(0x0003)).has_value());
    BOOST_CHECK(IsConsensusAlgorithm(PQCAlgorithm::ML_DSA_65));
    BOOST_CHECK(!IsConsensusAlgorithm(PQCAlgorithm::UNKNOWN));
    BOOST_CHECK(!IsConsensusAlgorithm(PQCAlgorithm::TEST_XOR));
}

BOOST_AUTO_TEST_CASE(registry)
{
    RegisterDefaultPQCBackends();
    const auto names{ListPQCBackends()};
    BOOST_CHECK(std::find(names.begin(), names.end(), "test") != names.end());
    auto test{GetPQCBackend("test")};
    BOOST_REQUIRE(test);
    BOOST_CHECK(test->IsTestOnly());
    BOOST_CHECK(test->Supports(PQCAlgorithm::TEST_XOR));
    BOOST_CHECK(!test->Supports(PQCAlgorithm::ML_DSA_65));
    BOOST_CHECK(GetPQCBackend("no-such-backend") == nullptr);
#ifdef HAVE_LIBOQS
    BOOST_CHECK(std::find(names.begin(), names.end(), "liboqs") != names.end());
    auto oqs{GetPQCBackend("liboqs")};
    BOOST_REQUIRE(oqs);
    BOOST_CHECK(!oqs->IsTestOnly());
    BOOST_CHECK(oqs->Supports(PQCAlgorithm::ML_DSA_65));
    BOOST_TEST_MESSAGE(std::string("liboqs version: ") + OqsBackend::OqsVersion());
#else
    BOOST_TEST_MESSAGE("liboqs backend not compiled; production PQC tests skipped (see PROOFS/pqc)");
#endif
}

BOOST_AUTO_TEST_CASE(envelopes)
{
    // Valid round-trips (TEST_XOR sizes: pub 32, sig 64).
    std::vector<unsigned char> raw_pub(32, 0x11), raw_sig(64, 0x22);
    auto enc_pub{EncodePubKeyBlob(PQCAlgorithm::TEST_XOR, raw_pub)};
    BOOST_REQUIRE(enc_pub.has_value());
    BOOST_CHECK_EQUAL(enc_pub->size(), 35U);
    auto dec_pub{DecodePubKeyBlob(*enc_pub)};
    BOOST_REQUIRE(dec_pub.has_value());
    BOOST_CHECK(dec_pub->first == PQCAlgorithm::TEST_XOR);
    BOOST_CHECK(dec_pub->second == raw_pub);
    auto enc_sig{EncodeSigBlob(PQCAlgorithm::TEST_XOR, raw_sig)};
    BOOST_REQUIRE(enc_sig.has_value());
    auto dec_sig{DecodeSigBlob(*enc_sig)};
    BOOST_REQUIRE(dec_sig.has_value());
    BOOST_CHECK(dec_sig->second == raw_sig);
    // Wrong version.
    auto bad_ver{*enc_pub};
    bad_ver[0] = 2;
    BOOST_CHECK(DecodePubKeyBlob(bad_ver).error() == PQCError::BAD_VERSION);
    // Truncated / oversized.
    BOOST_CHECK(DecodePubKeyBlob(std::span<const unsigned char>{enc_pub->data(), 10}).error() == PQCError::BAD_SIZE);
    auto over{*enc_pub};
    over.push_back(0x00);
    BOOST_CHECK(DecodePubKeyBlob(over).error() == PQCError::BAD_SIZE);
    BOOST_CHECK(DecodePubKeyBlob({}).error() == PQCError::BAD_ENCODING);
    // Unknown algorithm id.
    auto bad_algo{*enc_pub};
    bad_algo[1] = 0x03;
    bad_algo[2] = 0x00;
    BOOST_CHECK(DecodePubKeyBlob(bad_algo).error() == PQCError::UNKNOWN_ALGORITHM);
    // Wrong raw size at encode time.
    BOOST_CHECK(EncodePubKeyBlob(PQCAlgorithm::TEST_XOR, raw_sig).error() == PQCError::BAD_SIZE);
    BOOST_CHECK(EncodeSigBlob(PQCAlgorithm::UNKNOWN, raw_sig).error() == PQCError::UNKNOWN_ALGORITHM);
}

BOOST_AUTO_TEST_CASE(p2pq_script)
{
    uint256 keyid;
    CSHA256().Write((const unsigned char*)"test-keyid", 10).Finalize(keyid.begin());
    const CScript script{BuildP2PQScript(PQCAlgorithm::ML_DSA_65, keyid)};
    BOOST_REQUIRE_EQUAL(script.size(), 37U);
    BOOST_CHECK_EQUAL(script[0], OP_1);
    BOOST_CHECK_EQUAL(script[1], 35);
    auto info{ParseP2PQScript(script)};
    BOOST_REQUIRE(info.has_value());
    BOOST_CHECK_EQUAL(info->version, PQC_ENVELOPE_VERSION);
    BOOST_CHECK(info->algo == PQCAlgorithm::ML_DSA_65);
    BOOST_CHECK(info->keyid == keyid);
    // Malformed scripts.
    BOOST_CHECK(ParseP2PQScript(CScript()).error() == PQCError::BAD_ENCODING);
    BOOST_CHECK(ParseP2PQScript(CScript() << OP_1).error() == PQCError::BAD_ENCODING);
    CScript wrong_op{script};
    wrong_op[0] = OP_2;
    BOOST_CHECK(ParseP2PQScript(wrong_op).error() == PQCError::BAD_ENCODING);
    // TEST_XOR script can never parse as consensus-valid.
    const CScript test_script{BuildP2PQScript(PQCAlgorithm::TEST_XOR, keyid)};
    BOOST_CHECK(ParseP2PQScript(test_script).error() == PQCError::TEST_ONLY);
    // Commitment binding.
    std::vector<unsigned char> raw_pub(1952, 0x33);
    BOOST_CHECK(VerifyP2PQCommitment(*info, raw_pub) == PQCError::WRONG_KEY);
    // (Positive commitment check runs against the real backend below.)
}

BOOST_AUTO_TEST_CASE(test_backend_vectors)
{
    RegisterDefaultPQCBackends();
    auto be{GetPQCBackend("test")};
    BOOST_REQUIRE(be);
    const std::vector<unsigned char> msg1{'h', 'e', 'l', 'l', 'o'};
    const std::vector<unsigned char> msg2{'w', 'o', 'r', 'l', 'd'};
    auto k1{be->Generate(PQCAlgorithm::TEST_XOR)};
    auto k2{be->Generate(PQCAlgorithm::TEST_XOR)};
    BOOST_REQUIRE(k1.has_value() && k2.has_value());
    BOOST_CHECK(k1->pubkey != k2->pubkey); // distinct keys
    BOOST_CHECK(be->Generate(PQCAlgorithm::ML_DSA_65).error() == PQCError::UNSUPPORTED_ALGORITHM);
    auto sig{be->Sign(*k1, msg1)};
    BOOST_REQUIRE(sig.has_value());
    BOOST_CHECK_EQUAL(sig->bytes.size(), 64U);
    BOOST_CHECK(be->Verify(k1->pubkey, msg1, *sig) == PQCError::OK);
    // Negative battery (§52).
    BOOST_CHECK(be->Verify(k1->pubkey, msg2, *sig) == PQCError::VERIFY_FAILED); // modified message
    auto mut_sig{*sig};
    mut_sig.bytes[0] ^= 0x01;
    BOOST_CHECK(be->Verify(k1->pubkey, msg1, mut_sig) == PQCError::VERIFY_FAILED); // modified signature
    BOOST_CHECK(be->Verify(k2->pubkey, msg1, *sig) == PQCError::VERIFY_FAILED);   // wrong public key
    PQCSignature wrong_algo{PQCAlgorithm::ML_DSA_65, sig->bytes};
    BOOST_CHECK(be->Verify(k1->pubkey, msg1, wrong_algo) == PQCError::WRONG_KEY); // wrong algorithm
    PQCSignature truncated{PQCAlgorithm::TEST_XOR, std::vector<unsigned char>(sig->bytes.begin(), sig->bytes.begin() + 32)};
    BOOST_CHECK(be->Verify(k1->pubkey, msg1, truncated) == PQCError::BAD_SIZE);   // truncated
    PQCSignature oversized{PQCAlgorithm::TEST_XOR, sig->bytes};
    oversized.bytes.push_back(0x00);
    BOOST_CHECK(be->Verify(k1->pubkey, msg1, oversized) == PQCError::BAD_SIZE);   // oversized
    PQCKeyPair bad_key;
    BOOST_CHECK(be->Sign(bad_key, msg1).error() == PQCError::WRONG_KEY);          // malformed key
    // Full P2PQ spend path with the test backend's own sizes.
    PQCKeyPair tk;
    tk.algo = PQCAlgorithm::TEST_XOR;
    tk.pubkey.resize(32, 0x44);
    uint256 kid;
    CSHA256().Write(tk.pubkey.data(), tk.pubkey.size()).Finalize(kid.begin());
    // NOTE: TEST_XOR scripts never parse as consensus (asserted above); this
    // exercises only the commitment math, not consensus acceptance.
    P2PQInfo info;
    info.version = PQC_ENVELOPE_VERSION;
    info.algo = PQCAlgorithm::TEST_XOR;
    info.keyid = kid;
    BOOST_CHECK(VerifyP2PQCommitment(info, tk.pubkey) == PQCError::OK);
}

#ifdef HAVE_LIBOQS
BOOST_AUTO_TEST_CASE(p2pq_spend_e2e)
{
    // Full consensus-path spend: P2PQ prevout -> witness [sig,puk] verified
    // through VerifyScript (BIP143 SIGHASH_ALL binding). No mocks: real
    // liboqs signatures over a real transaction sighash.
    RegisterDefaultPQCBackends();
    auto be{GetPQCBackend("liboqs")};
    BOOST_REQUIRE(be);
    auto kp{be->Generate(PQCAlgorithm::ML_DSA_65)};
    BOOST_REQUIRE(kp.has_value());
    uint256 keyid;
    CSHA256().Write(kp->pubkey.data(), kp->pubkey.size()).Finalize(keyid.begin());
    const CScript spk{BuildP2PQScript(PQCAlgorithm::ML_DSA_65, keyid)};
    constexpr CAmount kPrevAmount{100};
    CMutableTransaction mtx;
    mtx.vin.resize(1);
    mtx.vin[0].prevout = COutPoint(Txid(), 0);
    mtx.vout.resize(1);
    mtx.vout[0].nValue = 90;
    mtx.vout[0].scriptPubKey = CScript() << OP_TRUE;
    const CTransaction tx(mtx);
    const uint256 sighash{SignatureHash(spk, tx, 0, SIGHASH_ALL, kPrevAmount, SigVersion::WITNESS_V0)};
    auto sig{be->Sign(*kp, {sighash.begin(), sighash.end()})};
    BOOST_REQUIRE(sig.has_value());
    auto sig_blob{EncodeSigBlob(PQCAlgorithm::ML_DSA_65, sig->bytes)};
    auto key_blob{EncodePubKeyBlob(PQCAlgorithm::ML_DSA_65, kp->pubkey)};
    BOOST_REQUIRE(sig_blob.has_value() && key_blob.has_value());
    CScriptWitness witness;
    witness.stack = {*sig_blob, *key_blob};
    TransactionSignatureChecker checker(&tx, 0, kPrevAmount, MissingDataBehavior::ASSERT_FAIL);
    ScriptError err{SCRIPT_ERR_OK};
    BOOST_CHECK_MESSAGE(VerifyScript(CScript(), spk, &witness,
                                     SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_WITNESS,
                                     checker, &err),
                        ScriptErrorString(err));
    // Mutated signature must fail consensus validation.
    auto mut_sig{*sig};
    mut_sig.bytes[7] ^= 0x01;
    auto mut_sig_blob{EncodeSigBlob(PQCAlgorithm::ML_DSA_65, mut_sig.bytes)};
    BOOST_REQUIRE(mut_sig_blob.has_value());
    CScriptWitness bad_witness;
    bad_witness.stack = {*mut_sig_blob, *key_blob};
    BOOST_CHECK(!VerifyScript(CScript(), spk, &bad_witness,
                              SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_WITNESS,
                              checker, &err));
    // Wrong amount changes the sighash: must fail.
    TransactionSignatureChecker bad_amount_checker(&tx, 0, kPrevAmount + 1, MissingDataBehavior::ASSERT_FAIL);
    BOOST_CHECK(!VerifyScript(CScript(), spk, &witness,
                              SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_WITNESS,
                              bad_amount_checker, &err));
    // Empty / malformed witness must fail closed.
    CScriptWitness empty_witness;
    BOOST_CHECK(!VerifyScript(CScript(), spk, &empty_witness,
                              SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_WITNESS,
                              checker, &err));
    CScriptWitness short_witness;
    short_witness.stack = {*sig_blob};
    BOOST_CHECK(!VerifyScript(CScript(), spk, &short_witness,
                              SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_WITNESS,
                              checker, &err));
}
#endif

#ifdef HAVE_LIBOQS
BOOST_AUTO_TEST_CASE(oqs_backend_mldsa65_kat)
{
    // External conformance: NIST ACVP ML-DSA-sigVer-FIPS204 vector
    // (ML-DSA-65, external/pure interface, empty context — matching this
    // backend's FIPS default). Vector pinned in data/mldsa65_kat.json.
    // A bit-flipped derivative serves as the negative (documented, not ACVP).
    RegisterDefaultPQCBackends();
    auto be{GetPQCBackend("liboqs")};
    BOOST_REQUIRE_MESSAGE(be, "liboqs backend not registered");
    UniValue vectors{read_json(json_tests::mldsa65_kat)};
    BOOST_REQUIRE_EQUAL(vectors.size(), 1U);
    const auto& v{vectors[0]};
    const auto pk{ParseHex<unsigned char>(v["pk"].get_str())};
    const auto msg{ParseHex<unsigned char>(v["message"].get_str())};
    const auto sigbytes{ParseHex<unsigned char>(v["signature"].get_str())};
    BOOST_REQUIRE_EQUAL(pk.size(), 1952U);
    BOOST_REQUIRE_EQUAL(sigbytes.size(), 3309U);
    const PQCSignature sig{PQCAlgorithm::ML_DSA_65, sigbytes};
    BOOST_REQUIRE_MESSAGE(v["testPassed"].get_bool(), "fixture must be a passing KAT");
    BOOST_CHECK_MESSAGE(be->Verify(pk, msg, sig) == PQCError::OK, "KAT verify failed");
    auto mut{sigbytes};
    mut[0] ^= 0x01;
    const PQCSignature mutsig{PQCAlgorithm::ML_DSA_65, mut};
    BOOST_CHECK_MESSAGE(be->Verify(pk, msg, mutsig) == PQCError::VERIFY_FAILED, "KAT derivative negative failed");
}
#endif

#ifdef HAVE_LIBOQS
BOOST_AUTO_TEST_CASE(oqs_backend_mldsa65)
{
    RegisterDefaultPQCBackends();
    auto be{GetPQCBackend("liboqs")};
    BOOST_REQUIRE_MESSAGE(be, "liboqs backend not registered");
    // Sizes cross-checked against FIPS 204 parameters.
    auto kp{be->Generate(PQCAlgorithm::ML_DSA_65)};
    BOOST_REQUIRE(kp.has_value());
    BOOST_CHECK_EQUAL(kp->pubkey.size(), 1952U);
    BOOST_CHECK_EQUAL(kp->privkey.size(), 4032U);
    const std::vector<unsigned char> msg{'q', 'u', 'a', 'n', 't', 'b', 't', 'c'};
    auto sig{be->Sign(*kp, msg)};
    BOOST_REQUIRE(sig.has_value());
    BOOST_CHECK_EQUAL(sig->bytes.size(), 3309U);
    BOOST_CHECK(be->Verify(kp->pubkey, msg, *sig) == PQCError::OK);
    // Negative battery on the production path.
    const std::vector<unsigned char> msg2{'t', 'a', 'm', 'p', 'e', 'r', 'e', 'd'};
    BOOST_CHECK(be->Verify(kp->pubkey, msg2, *sig) == PQCError::VERIFY_FAILED);
    auto mut{*sig};
    mut.bytes[100] ^= 0x01;
    BOOST_CHECK(be->Verify(kp->pubkey, msg, mut) == PQCError::VERIFY_FAILED);
    auto kp2{be->Generate(PQCAlgorithm::ML_DSA_65)};
    BOOST_REQUIRE(kp2.has_value());
    BOOST_CHECK(be->Verify(kp2->pubkey, msg, *sig) == PQCError::VERIFY_FAILED);
    auto trunc{*sig};
    trunc.bytes.resize(100);
    BOOST_CHECK(be->Verify(kp->pubkey, msg, trunc) == PQCError::BAD_SIZE);
    // Positive P2PQ commitment binding with a production key.
    uint256 kid;
    CSHA256().Write(kp->pubkey.data(), kp->pubkey.size()).Finalize(kid.begin());
    const CScript script{BuildP2PQScript(PQCAlgorithm::ML_DSA_65, kid)};
    auto info{ParseP2PQScript(script)};
    BOOST_REQUIRE(info.has_value());
    BOOST_CHECK(VerifyP2PQCommitment(*info, kp->pubkey) == PQCError::OK);
}
#endif

BOOST_AUTO_TEST_SUITE_END()
