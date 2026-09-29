// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>
#include <util/check.h>

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    if (params.fUseASERT) {
        // ASERT evaluates the tip (no retarget windows, no pblock dependency).
        return CalculateASERT(pindexLast, params);
    }
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then it MUST be a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 14 days worth of blocks
    int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;

    // Special difficulty rule for Testnet4
    if (params.enforce_BIP94) {
        // Here we use the first block of the difficulty period. This way
        // the real difficulty is always preserved in the first block as
        // it is not allowed to use the min-difficulty exception.
        int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
        const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
        bnNew.SetCompact(pindexFirst->nBits);
    } else {
        bnNew.SetCompact(pindexLast->nBits);
    }

    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

// Check that on difficulty adjustments, the new difficulty does not increase
// or decrease beyond the permitted limits.
bool PermittedDifficultyTransition(const Consensus::Params& params, int64_t height, uint32_t old_nbits, uint32_t new_nbits)
{
    if (params.fUseASERT) {
        // ASERT permits arbitrary single-step transitions under adversarial
        // timestamps, so only well-formedness (non-zero, within pow limit)
        // can be enforced here. Full rules are enforced by CheckProofOfWork
        // and contextual validation; headersync's work/commitment accounting
        // still bounds presync abuse. See DECISIONS D-016.
        arith_uint256 target;
        bool neg = false, over = false;
        target.SetCompact(new_nbits, &neg, &over);
        return !neg && !over && target != 0 && target <= UintToArith256(params.powLimit);
    }
    if (params.fPowAllowMinDifficultyBlocks) return true;

    if (height % params.DifficultyAdjustmentInterval() == 0) {
        int64_t smallest_timespan = params.nPowTargetTimespan/4;
        int64_t largest_timespan = params.nPowTargetTimespan*4;

        const arith_uint256 pow_limit = UintToArith256(params.powLimit);
        arith_uint256 observed_new_target;
        observed_new_target.SetCompact(new_nbits);

        // Calculate the largest difficulty value possible:
        arith_uint256 largest_difficulty_target;
        largest_difficulty_target.SetCompact(old_nbits);
        largest_difficulty_target *= largest_timespan;
        largest_difficulty_target /= params.nPowTargetTimespan;

        if (largest_difficulty_target > pow_limit) {
            largest_difficulty_target = pow_limit;
        }

        // Round and then compare this new calculated value to what is
        // observed.
        arith_uint256 maximum_new_target;
        maximum_new_target.SetCompact(largest_difficulty_target.GetCompact());
        if (maximum_new_target < observed_new_target) return false;

        // Calculate the smallest difficulty value possible:
        arith_uint256 smallest_difficulty_target;
        smallest_difficulty_target.SetCompact(old_nbits);
        smallest_difficulty_target *= smallest_timespan;
        smallest_difficulty_target /= params.nPowTargetTimespan;

        if (smallest_difficulty_target > pow_limit) {
            smallest_difficulty_target = pow_limit;
        }

        // Round and then compare this new calculated value to what is
        // observed.
        arith_uint256 minimum_new_target;
        minimum_new_target.SetCompact(smallest_difficulty_target.GetCompact());
        if (minimum_new_target > observed_new_target) return false;
    } else if (old_nbits != new_nbits) {
        return false;
    }
    return true;
}

unsigned int CalculateASERT(const CBlockIndex* pindexLast, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    // Integer-exact aserti3-2d (see pow.h). All arithmetic is int64 or
    // arith_uint256. Reachable inputs derive from uint32 timestamps and
    // int32 heights, so scaled intermediates stay far below int64 limits
    // (worst case ~2^57); the +-512 guards below additionally map any
    // theoretical extreme to the correct clamp.
    if (params.nASERTHalfLife <= 0) return UintToArith256(params.powLimit).GetCompact();
    const arith_uint256 pow_limit = UintToArith256(params.powLimit);
    const uint32_t max_bits = pow_limit.GetCompact();

    arith_uint256 anchor_target;
    bool neg = false, over = false;
    anchor_target.SetCompact(params.nASERTAnchorBits, &neg, &over);
    if (neg || over || anchor_target == 0 || anchor_target > pow_limit) {
        anchor_target = pow_limit; // invalid anchor config: stay total, never widen
    }

    const int64_t time_delta = int64_t(pindexLast->GetBlockTime()) - params.nASERTAnchorTime;
    const int64_t height_delta = int64_t(pindexLast->nHeight) - params.nASERTAnchorHeight;
    // trunc((time_delta - spacing*(height_delta+1)) * 65536 / halflife),
    // factored to avoid any intermediate overflow (see DECISIONS D-015).
    const int64_t span = time_delta - params.nPowTargetSpacing * (height_delta + 1);
    const int64_t exponent = span / params.nASERTHalfLife * 65536 +
                             (span % params.nASERTHalfLife) * 65536 / params.nASERTHalfLife;
    // floor(exponent / 65536) without relying on shift semantics:
    const int64_t num_shifts = exponent >= 0 ? exponent >> 16 : -((-exponent + 65535) / 65536);
    const int64_t exp_rem = exponent - num_shifts * 65536;
    // Cubic 2^x approximation (fixed point, radix 2^16). The raw terms
    // exceed int64 (up to ~2^64), so this is computed in arith_uint256;
    // the result is proven < 2^18, hence exact via GetLow64.
    const int64_t factor = int64_t(((arith_uint256(195766423245049ULL) * uint64_t(exp_rem) +
                                     arith_uint256(971821376ULL) * uint64_t(exp_rem) * uint64_t(exp_rem) +
                                     arith_uint256(5127ULL) * uint64_t(exp_rem) * uint64_t(exp_rem) * uint64_t(exp_rem) +
                                     (arith_uint256(1) << 47)) >> 48).GetLow64() + 65536);
    // Sound early-outs: anchor <= pow_limit < 2^224 and factor < 2^18, so
    // |num_shifts| >= 512 provably lands beyond the clamps below.
    if (num_shifts >= 512) return max_bits;
    if (num_shifts <= -512) return arith_uint256(1).GetCompact();
    arith_uint256 next_target = anchor_target * arith_uint256(uint64_t(factor));
    if (num_shifts < 0) {
        next_target >>= (unsigned int)(-num_shifts);
    } else {
        next_target <<= (unsigned int)num_shifts;
    }
    next_target >>= 16;
    if (next_target == 0) return arith_uint256(1).GetCompact();
    if (next_target > pow_limit) return max_bits;
    return next_target.GetCompact();
}

// Bypasses the actual proof of work check during fuzz testing with a simplified validation checking whether
// the most significant bit of the last byte of the hash is set.
bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params){
    if (EnableFuzzDeterminism()) return (hash.data()[31] & 0x80) == 0;
    return CheckProofOfWorkImpl(hash, nBits, params);
}

std::optional<arith_uint256> DeriveTarget(unsigned int nBits, const uint256 pow_limit)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(pow_limit))
        return {};

    return bnTarget;
}

bool CheckProofOfWorkImpl(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    auto bnTarget{DeriveTarget(nBits, params.powLimit)};
    if (!bnTarget) return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
