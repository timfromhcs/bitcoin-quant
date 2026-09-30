// Copyright (c) 2026-present The QuantBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <rpc/register.h>
#include <rpc/server.h>
#include <rpc/server_util.h>
#include <rpc/util.h>
#include <crypto/hex_base.h>
#include <crypto/sha256.h>
#include <storage/proof.h>
#include <uint256.h>
#include <util/strencodings.h>

#include <univalue.h>

#include <algorithm>
#include <cstring>
#include <span>
#include <string>
#include <vector>

namespace {
//! Raw byte-order hex of a uint256 (NOT GetHex: that display-reverses).
std::string RawHex256(const uint256& h)
{
    return HexStr(std::span<const uint8_t>(h.begin(), h.end()));
}
//! Max content accepted over RPC (1 MiB = 16 chunks at 64 KiB).
//! Bounds CPU/memory per call; larger content is chunked client-side.
static constexpr size_t MAX_STORAGE_RPC_CONTENT = 1 << 20;
//! Max challenge samples per call (each is 2 SHA256d; generous but bounded).
static constexpr uint32_t MAX_CHALLENGE_SAMPLES = 1024;

//! Parse raw-byte-order hex into uint256 (NO display reversal: storage
//! roots/paths are byte strings, not chain hashes — documented in help).
bool ParseRaw256(const UniValue& v, uint256& out, std::string& err)
{
    if (!v.isStr()) {
        err = "Expected hex string";
        return false;
    }
    const std::vector<unsigned char> b{ParseHex(v.get_str())};
    if (b.size() != 32) {
        err = "Expected 32-byte hex string (raw byte order, not display-reversed)";
        return false;
    }
    std::memcpy(out.begin(), b.data(), 32);
    return true;
}

bool ParseContent(const UniValue& v, std::vector<unsigned char>& out, std::string& err)
{
    if (!v.isStr()) {
        err = "Expected hex string";
        return false;
    }
    out = ParseHex(v.get_str());
    if (out.empty()) {
        err = "Empty content has no commitment";
        return false;
    }
    if (out.size() > MAX_STORAGE_RPC_CONTENT) {
        err = strprintf("Content too large (%u bytes > %u)", out.size(), MAX_STORAGE_RPC_CONTENT);
        return false;
    }
    return true;
}

bool ParseEpoch(const UniValue& v, uint64_t& out, std::string& err)
{
    if (v.isNum()) {
        const int64_t n{v.getInt<int64_t>()};
        if (n < 0) {
            err = "Negative epoch";
            return false;
        }
        out = static_cast<uint64_t>(n);
        return true;
    }
    if (v.isStr()) {
        const std::string s{v.get_str()};
        if (s.empty() || s.size() > 20 ||
            !std::all_of(s.begin(), s.end(), [](char c) { return c >= '0' && c <= '9'; })) {
            err = "Epoch must be a non-negative integer (number or numeric string)";
            return false;
        }
        try {
            out = static_cast<uint64_t>(std::stoull(s));
        } catch (...) {
            err = "Epoch out of range";
            return false;
        }
        return true;
    }
    err = "Epoch must be a non-negative integer (number or numeric string)";
    return false;
}

std::vector<uint256> ChunkLeaves(const std::vector<unsigned char>& content)
{
    std::vector<uint256> leaves;
    for (size_t off = 0; off < content.size(); off += storage::CHUNK_SIZE) {
        const size_t n{std::min<size_t>(storage::CHUNK_SIZE, content.size() - off)};
        leaves.push_back(storage::ChunkHash(content.data() + off, n));
    }
    return leaves;
}

std::string VerifyReason(storage::VerifyResult r)
{
    switch (r) {
    case storage::VerifyResult::OK: return "ok";
    case storage::VerifyResult::MALFORMED: return "malformed-proof";
    case storage::VerifyResult::BAD_CHUNK_SIZE: return "bad-chunk-size";
    case storage::VerifyResult::BAD_PATH: return "bad-path";
    case storage::VerifyResult::PATH_LENGTH_MISMATCH: return "path-length-mismatch";
    case storage::VerifyResult::ROOT_MISMATCH: return "root-mismatch";
    }
    return "unknown";
}

static RPCMethod storagecommit()
{
    return RPCMethod{
        "storagecommit",
        "Commit content to a storage root (QuantBTC storage protocol v1).\n"
        "Chunks the content into 65536-byte pieces, hashes each with SHA256d,\n"
        "and folds them into a binary Merkle root (Bitcoin duplicate-odd).\n"
        "Pure computation: no chain state, no wallet, deterministic.\n"
        "NOTE: all hashes are RAW byte order hex (not display-reversed).\n",
        {
            {"content", RPCArg::Type::STR_HEX, RPCArg::Optional::OMITTED, "Content bytes (hex, 1 byte..1 MiB)"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::STR_HEX, "root", "Content root (raw byte order)"},
                {RPCResult::Type::NUM, "chunks", "Number of chunks"},
                {RPCResult::Type::NUM, "chunk_size", "Chunk size (65536)"},
                {RPCResult::Type::ARR, "chunk_hashes", "Per-chunk SHA256d (raw byte order)",
                 {{RPCResult::Type::STR_HEX, "", ""}}},
            }},
        RPCExamples{HelpExampleCli("storagecommit", "\"deadbeef\"") +
                    HelpExampleRpc("storagecommit", "\"deadbeef\"")},
        [](const RPCMethod& self, const JSONRPCRequest& request) -> UniValue {
            std::vector<unsigned char> content;
            std::string err;
            if (!ParseContent(request.params[0], content, err)) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, err);
            }
            const std::vector<uint256> leaves{ChunkLeaves(content)};
            const uint256 root{storage::MerkleRoot(leaves)};
            UniValue result(UniValue::VOBJ);
            result.pushKV("root", RawHex256(root));
            result.pushKV("chunks", static_cast<int64_t>(leaves.size()));
            result.pushKV("chunk_size", static_cast<int64_t>(storage::CHUNK_SIZE));
            UniValue arr(UniValue::VARR);
            for (const uint256& h : leaves) arr.push_back(RawHex256(h));
            result.pushKV("chunk_hashes", arr);
            return result;
        },
    };
}

static RPCMethod storagechallenge()
{
    return RPCMethod{
        "storagechallenge",
        "Derive deterministic challenge indices for a storage audit epoch.\n"
        "seed = SHA256d(root || provider_id || BE64(epoch)); indices are\n"
        "seed-hashes mod n_leaves. The provider cannot grind: entropy binds\n"
        "content (root), prover (provider id) and time (epoch).\n"
        "Pure computation, deterministic.\n",
        {
            {"root", RPCArg::Type::STR_HEX, RPCArg::Optional::OMITTED, "Content root (32-byte raw hex)"},
            {"provider", RPCArg::Type::STR_HEX, RPCArg::Optional::OMITTED, "Provider id (32-byte raw hex)"},
            {"epoch", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "Audit epoch (non-negative integer)"},
            {"nleaves", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "Chunk count (>= 1)"},
            {"samples", RPCArg::Type::NUM, RPCArg::Default{16}, "Sample count (1..1024)"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::ARR, "indices", "Challenged chunk indices",
                 {{RPCResult::Type::NUM, "", ""}}},
            }},
        RPCExamples{HelpExampleCli("storagechallenge", "\"<root>\" \"<provider>\" 7 3") +
                    HelpExampleRpc("storagechallenge", "\"<root>\", \"<provider>\", 7, 3")},
        [](const RPCMethod& self, const JSONRPCRequest& request) -> UniValue {
            uint256 root, provider;
            std::string err;
            if (!ParseRaw256(request.params[0], root, err)) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "root: " + err);
            }
            if (!ParseRaw256(request.params[1], provider, err)) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "provider: " + err);
            }
            uint64_t epoch{0};
            if (!ParseEpoch(request.params[2], epoch, err)) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "epoch: " + err);
            }
            if (!request.params[3].isNum() || request.params[3].getInt<int64_t>() < 1) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "nleaves must be >= 1");
            }
            const uint32_t nleaves{static_cast<uint32_t>(std::min<int64_t>(request.params[3].getInt<int64_t>(), 0xFFFFFFFFLL))};
            uint32_t k{storage::CHALLENGE_SAMPLES};
            if (!request.params[4].isNull()) {
                if (!request.params[4].isNum() || request.params[4].getInt<int64_t>() < 1 ||
                    request.params[4].getInt<int64_t>() > MAX_CHALLENGE_SAMPLES) {
                    throw JSONRPCError(RPC_INVALID_PARAMETER, "samples must be 1..1024");
                }
                k = static_cast<uint32_t>(request.params[4].getInt<int64_t>());
            }
            UniValue result(UniValue::VOBJ);
            UniValue arr(UniValue::VARR);
            for (uint32_t i : storage::DeriveIndices(root, provider, epoch, nleaves, k)) {
                arr.push_back(static_cast<int64_t>(i));
            }
            result.pushKV("indices", arr);
            return result;
        },
    };
}

static RPCMethod storageprove()
{
    return RPCMethod{
        "storageprove",
        "Build a Merkle inclusion proof for one chunk of the given content.\n"
        "Returns the chunk bytes plus the sibling path (bottom-up) and the\n"
        "strict versioned proof envelope for transport.\n"
        "Pure computation, deterministic.\n",
        {
            {"content", RPCArg::Type::STR_HEX, RPCArg::Optional::OMITTED, "Content bytes (hex, 1 byte..1 MiB)"},
            {"index", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "Chunk index (0-based)"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::NUM, "index", "Chunk index"},
                {RPCResult::Type::STR_HEX, "chunk", "Chunk bytes"},
                {RPCResult::Type::ARR, "path", "Sibling hashes bottom-up (raw hex)",
                 {{RPCResult::Type::STR_HEX, "", ""}}},
                {RPCResult::Type::STR_HEX, "envelope", "Versioned proof envelope for storageverify"},
            }},
        RPCExamples{HelpExampleCli("storageprove", "\"deadbeef\" 0") +
                    HelpExampleRpc("storageprove", "\"deadbeef\", 0")},
        [](const RPCMethod& self, const JSONRPCRequest& request) -> UniValue {
            std::vector<unsigned char> content;
            std::string err;
            if (!ParseContent(request.params[0], content, err)) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, err);
            }
            if (!request.params[1].isNum() || request.params[1].getInt<int64_t>() < 0) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "index must be >= 0");
            }
            const size_t nchunks{(content.size() + storage::CHUNK_SIZE - 1) / storage::CHUNK_SIZE};
            const auto want{static_cast<uint64_t>(request.params[1].getInt<int64_t>())};
            if (want >= nchunks) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, strprintf("index out of range (0..%u)", nchunks - 1));
            }
            // Fold levels while tracking the wanted leaf's siblings.
            std::vector<uint256> level{ChunkLeaves(content)};
            std::vector<uint256> path;
            size_t idx{static_cast<size_t>(want)};
            while (level.size() > 1) {
                if (level.size() % 2 == 1) level.push_back(level.back());
                path.push_back(level[idx ^ 1]);
                std::vector<uint256> next;
                next.reserve(level.size() / 2);
                std::vector<unsigned char> buf;
                buf.reserve(64);
                for (size_t i = 0; i < level.size(); i += 2) {
                    buf.clear();
                    buf.insert(buf.end(), level[i].begin(), level[i].end());
                    buf.insert(buf.end(), level[i + 1].begin(), level[i + 1].end());
                    uint256 h;
                    unsigned char tmp[CSHA256::OUTPUT_SIZE];
                    CSHA256().Write(buf.data(), buf.size()).Finalize(tmp);
                    CSHA256().Write(tmp, sizeof(tmp)).Finalize(h.begin());
                    next.push_back(h);
                }
                level = std::move(next);
                idx /= 2;
            }
            const size_t off{static_cast<size_t>(want) * storage::CHUNK_SIZE};
            const size_t n{std::min<size_t>(storage::CHUNK_SIZE, content.size() - off)};
            storage::Proof pr;
            pr.index = static_cast<uint32_t>(want);
            pr.chunk.assign(content.begin() + off, content.begin() + off + n);
            pr.path = std::move(path);
            UniValue result(UniValue::VOBJ);
            result.pushKV("index", static_cast<int64_t>(pr.index));
            result.pushKV("chunk", HexStr(pr.chunk));
            UniValue arr(UniValue::VARR);
            for (const uint256& h : pr.path) arr.push_back(RawHex256(h));
            result.pushKV("path", arr);
            result.pushKV("envelope", HexStr(storage::EncodeProof(pr)));
            return result;
        },
    };
}

static RPCMethod storageverify()
{
    return RPCMethod{
        "storageverify",
        "Verify a storage proof envelope against a content root.\n"
        "Returns valid=false with a machine-readable reason on ANY failure\n"
        "(malformed envelope, bad chunk, bad path, root mismatch) — never\n"
        "throws for proof content. Fail-closed by construction.\n",
        {
            {"root", RPCArg::Type::STR_HEX, RPCArg::Optional::OMITTED, "Content root (32-byte raw hex)"},
            {"proof", RPCArg::Type::STR_HEX, RPCArg::Optional::OMITTED, "Proof envelope from storageprove"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::BOOL, "valid", "Proof valid against root"},
                {RPCResult::Type::STR, "reason", "ok | malformed-proof | bad-chunk-size | bad-path | path-length-mismatch | root-mismatch"},
            }},
        RPCExamples{HelpExampleCli("storageverify", "\"<root>\" \"<envelope>\"") +
                    HelpExampleRpc("storageverify", "\"<root>\", \"<envelope>\"")},
        [](const RPCMethod& self, const JSONRPCRequest& request) -> UniValue {
            uint256 root;
            std::string err;
            if (!ParseRaw256(request.params[0], root, err)) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "root: " + err);
            }
            if (!request.params[1].isStr()) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "proof: Expected hex string");
            }
            const std::vector<unsigned char> blob{ParseHex(request.params[1].get_str())};
            storage::Proof pr;
            storage::VerifyResult r{storage::VerifyResult::MALFORMED};
            if (storage::DecodeProof(blob.data(), blob.size(), pr)) {
                r = storage::Verify(root, pr);
            }
            UniValue result(UniValue::VOBJ);
            result.pushKV("valid", r == storage::VerifyResult::OK);
            result.pushKV("reason", VerifyReason(r));
            return result;
        },
    };
}
} // namespace

void RegisterStorageRPCCommands(CRPCTable& t)
{
    static const CRPCCommand commands[]{
        {"storage", &storagecommit},
        {"storage", &storagechallenge},
        {"storage", &storageprove},
        {"storage", &storageverify},
    };
    for (const auto& c : commands) {
        t.appendCommand(c.name, &c);
    }
}
