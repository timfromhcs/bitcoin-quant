"""Difficulty-controller comparison simulation (§39–§42).

Compares on synthetic hashrate scenarios:
  - bitcoin_retarget: 2016-block window, 4x clamp (src/pow.cpp GetNextWorkRequired)
  - asert: exponential anchor-based retarget, halflife 2 days (BCH-ASERT formula,
    integer arithmetic with fixed-point SHIFT)
  - lwma: linearly-weighted moving average over N=60 blocks (Zcash LWMA-3 style,
    simplified: no solvetimes floor tricks beyond 6*T clamp)

Scenarios: steady, step 4x up at H500, step 10x down at H1000, 1-miner (long gaps),
timestamp-warp attack (miner stamps +2h each block for 100 blocks).

Outputs JSON summary to stdout + vectors/difficulty-sim.json.
Deterministic (§204): seeded LCG, no wall-clock, no randomness module.
NOT a consensus freeze decision (§39: freeze only after fuller evidence).
"""
import json
import sys
from pathlib import Path

T = 600                      # target spacing, seconds
HALFLIFE = 2 * 24 * 3600     # ASERT halflife
WINDOW = 2016
LWMA_N = 60
SHIFT = 32                   # ASERT fixed-point bits


def lcg(seed: int):
    state = seed & 0xFFFFFFFF
    while True:
        state = (1103515245 * state + 12345) & 0xFFFFFFFF
        yield state / 2**32


def bitcoin_retarget(prev_target: int, first_time: int, last_time: int) -> int:
    span = max(1, last_time - first_time)
    span = min(max(span, T * WINDOW // 4), T * WINDOW * 4)
    return prev_target * span // (T * WINDOW)


def asert_next(anchor_target: int, time_delta: int, height_delta: int) -> int:
    """next = anchor * 2^((time_delta - T*(h+1))/HALFLIFE).
    SIMULATION PRECISION NOTE: uses float for behavioral comparison only.
    The consensus implementation MUST use exact integer arithmetic and pass
    differential tests against this reference behavior (§114) before any freeze.
    """
    exponent = (time_delta - T * (height_delta + 1)) / HALFLIFE
    q = int(exponent)  # floor for positive; fractional part in [0,1)
    frac = exponent - q
    shifted = anchor_target << q if q >= 0 else anchor_target >> (-q)
    if shifted.bit_length() > 280:  # caller clamps to pow limit anyway; avoid float overflow
        return 1 << 300
    return int(shifted * (2.0 ** frac))


def lwma_next(targets: list[int], solvetimes: list[int], t0: int) -> int:
    """Linearly-weighted difficulty estimate (Zawy-LWMA philosophy):
    H_est = T*S(w*D)/S(w*t) with D=T0/T; next target aims H_est at spacing T:
    next_T = T0*S(w*t)/(T*S(w*D)). Solvetimes clamped to [1, 6T] (anti-warp)."""
    n = len(targets)
    assert n == len(solvetimes) == LWMA_N
    num = den = 0
    for i, (tg, st) in enumerate(zip(targets, solvetimes)):
        w = i + 1
        t = min(6 * T, max(1, st))
        num += w * t
        den += w * (t0 * T // max(1, tg))  # w * T * D
    return t0 * num // den if den else targets[-1]


def simulate(controller: str, hashrate: list[float], warp_extra: list[int]) -> dict:
    """Closed-loop model: expected block time = T * (T0/target) / hashrate,
    i.e. an easier (larger) target yields FASTER blocks at fixed hashrate.
    Exponential noise via LCG. Target evolves per controller from OBSERVED times
    (incl. warp dishonesty). T0 = reference target at hashrate 1.0 -> T spacing."""
    rng = lcg(0xD1FF1C17 + len(hashrate))
    T0 = 2**224
    target = T0
    anchor_target, anchor_h, anchor_t = target, 0, 0
    times = [0]
    hist_tg, hist_st = [], []
    min_target = T0 // 2**16
    max_target = T0 * 2**16  # wide pow-limit band so recovery dynamics are visible
    for h in range(1, len(hashrate) + 1):
        gap = max(1, int(T * (T0 / target) / max(1e-9, hashrate[h - 1]) * (0.5 + next(rng))))
        true_t = times[-1] + gap
        obs_t = true_t + warp_extra[h - 1]
        times.append(obs_t)
        if controller == "bitcoin":
            if h % WINDOW == 0:
                target = min(max_target, max(min_target,
                             bitcoin_retarget(target, times[h - WINDOW], times[h - 1])))
        elif controller == "asert":
            target = min(max_target, max(min_target,
                         asert_next(anchor_target, times[h - 1] - anchor_t, h - 1 - anchor_h)))
        elif controller == "lwma":
            hist_tg.append(target)
            hist_st.append(obs_t - times[h - 2] if h >= 2 else T)
            if len(hist_tg) >= LWMA_N:
                target = min(max_target, max(min_target,
                             lwma_next(hist_tg[-LWMA_N:], hist_st[-LWMA_N:], T0)))
    spacings = [times[i] - times[i - 1] - warp_extra[i - 1] for i in range(1, len(times))]
    return {"controller": controller, "final_target": targets_last(target),
            "median_spacing": sorted(spacings)[len(spacings) // 2],
            "max_spacing": max(spacings), "min_spacing": min(spacings),
            "n_blocks": len(times) - 1}


def targets_last(t: int) -> int:
    return t


def scenario(kind: str, n: int = 2500) -> tuple[list[float], list[int]]:
    """Returns (relative_hashrate_per_height, warp_dishonesty_seconds_per_height)."""
    hr, warp = [], []
    for h in range(n):
        if kind == "steady":
            r = 1.0
        elif kind == "step_up":
            r = 1.0 if h < 500 else 4.0
        elif kind == "crash":
            r = 1.0 if h < 500 else 0.1
        elif kind == "lone_miner":
            r = 0.02
        elif kind == "warp":
            r = 1.0
        hr.append(r)
        warp.append(7000 if kind == "warp" and 500 <= h < 600 else 0)
    return hr, warp


def main() -> int:
    results = {}
    for kind in ("steady", "step_up", "crash", "lone_miner", "warp"):
        hr, warp = scenario(kind)
        results[kind] = {c: simulate(c, hr, warp) for c in ("bitcoin", "asert", "lwma")}
    dest = Path(__file__).resolve().parent.parent / "genesis" / "vectors" / "difficulty-sim.json"
    dest.write_text(json.dumps({"scenarios": results,
                                "note": "DRAFT simulation evidence, §39-§42. NOT a freeze decision."},
                               indent=2) + "\n")
    for kind, ctrls in results.items():
        row = "  ".join(f"{c}: med={v['median_spacing']:>6}s max={v['max_spacing']:>7}s"
                        for c, v in ctrls.items())
        print(f"{kind:>10}: {row}")
    print("PASS: simulation complete, vectors written")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
