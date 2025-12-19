// exchange/market_replayer.hpp
#pragma once

#include "core/types.hpp"
#include "exchange/order.hpp"
#include "exchange/order_book.hpp"
#include "risk/risk.hpp"
#include "latency/timer.hpp"

#include <string>
#include <cstdio>

namespace nanomarket::exchange {

// MarketDataReplayer reads a deterministic CSV of events and feeds the OrderBook and RiskEngine.
// The replay is single-threaded and uses logical timestamps provided in the input or generated
// by a local counter. Execution outputs and risk state are written deterministically to a log file.

class MarketDataReplayer {
public:
    struct Config {
        const char* infile = nullptr;
        const char* outfile = "replay.log";
    };

    MarketDataReplayer(const Config& cfg, OrderBook& book, nanomarket::risk::RiskEngine& risk) noexcept;
    ~MarketDataReplayer();

    // Run replay to completion. Returns 0 on success.
    int run() noexcept;

private:
    Config cfg_;
    OrderBook& book_;
    nanomarket::risk::RiskEngine& risk_;

    // deterministic timestamp if not supplied by input
    core::Timestamp logical_ts_ = 1;

    // internal FILE* for deterministic logging (avoid iostream overhead ordering differences)
    FILE* logf_ = nullptr;

    // parse a CSV line into an Order. Returns true if parsed.
    bool parse_line(char* buf, Order& out) noexcept;
};

} // namespace nanomarket::exchange
