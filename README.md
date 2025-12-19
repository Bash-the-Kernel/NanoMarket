# NanoMarket

Production-minded, low-latency trading system skeleton in modern C++ (C++20).

Architecture: modular pipeline — MarketData → Exchange Simulator → Order Book → Matching Engine → Strategy → Risk → Order Router

Design priorities: determinism, minimal allocations on hot paths, cache-friendly data layout, clear ownership boundaries.

See docs/ for deeper notes.
# NanoMarket
A low-latency exchange simulator and market-making system written in modern C++, designed with production trading systems in mind.
