# NanoMarket

Production-minded, low-latency trading system skeleton in modern C++ (C++20).

Architecture: modular pipeline — MarketData → Exchange Simulator → Order Book → Matching Engine → Strategy → Risk → Order Router

Design priorities: determinism, minimal allocations on hot paths, cache-friendly data layout, clear ownership boundaries.

See docs/ for deeper notes.

## Determinism & Correctness Guarantees

- **Why determinism matters:** Trading systems require exact replayability for debugging, regulatory audits, and backtesting. Deterministic matching ensures identical inputs produce identical outputs bit-for-bit.
- **How this system enforces determinism:** Matching is single-threaded and uses preallocated data structures. Timestamps used in matching are provided by the producer or an internal deterministic sequence; the matching engine does not call wall-clock time. No unordered containers are iterated in hot paths.
- **How tests validate it:** Unit tests in `tests/` use fixed inputs and deterministic timestamps; they assert identical outcomes across repeated runs. The tests run in Release mode to ensure behavior remains correct without debug-only asserts.

# NanoMarket
A low-latency exchange simulator and market-making system written in modern C++, designed with production trading systems in mind.
