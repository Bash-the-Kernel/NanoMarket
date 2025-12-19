#include "exchange/market_replayer.hpp"
#include "core/types.hpp"
#include "exchange/order.hpp"

#include <cstring>
#include <cstdlib>

using namespace nanomarket::core;
namespace em = nanomarket::exchange;

em::MarketDataReplayer::MarketDataReplayer(const Config& cfg, OrderBook& book, nanomarket::risk::RiskEngine& risk) noexcept
    : cfg_(cfg), book_(book), risk_(risk), logical_ts_(1), logf_(nullptr) {
    if (fopen_s(&logf_, cfg_.outfile, "w") != 0) logf_ = nullptr;
}

em::MarketDataReplayer::~MarketDataReplayer() {
    if (logf_) std::fclose(logf_);
}

bool em::MarketDataReplayer::parse_line(char* buf, Order& out) noexcept {
    // CSV format: ORDER,id,side(B|S),price,qty[,ts]
    // Example: ORDER,1,B,10002,5,10
    // We operate in-place over the buffer to avoid allocations.
    char* ctx = nullptr;
    char* tok = strtok_s(buf, ",\n\r", &ctx);
    if (!tok) return false;
    if (std::strcmp(tok, "ORDER") != 0) return false;

    // id
    tok = strtok_s(nullptr, ",\n\r", &ctx); if (!tok) return false;
    out.id = static_cast<OrderId>(std::strtoull(tok, nullptr, 10));

    // side
    tok = strtok_s(nullptr, ",\n\r", &ctx); if (!tok) return false;
    out.side = (tok[0] == 'B') ? Side::Buy : Side::Sell;

    // price
    tok = strtok_s(nullptr, ",\n\r", &ctx); if (!tok) return false;
    out.price = static_cast<Price>(std::strtoll(tok, nullptr, 10));

    // qty
    tok = strtok_s(nullptr, ",\n\r", &ctx); if (!tok) return false;
    out.qty = static_cast<Qty>(std::strtol(tok, nullptr, 10));
    out.remaining = out.qty;

    // optional ts
    tok = strtok_s(nullptr, ",\n\r", &ctx);
    if (tok) {
        out.ts = static_cast<Timestamp>(std::strtoll(tok, nullptr, 10));
        logical_ts_ = out.ts + 1;
    } else {
        out.ts = logical_ts_++;
    }

    out.next = -1;
    return true;
}

int em::MarketDataReplayer::run() noexcept {
    if (!cfg_.infile) return -1;
    FILE* f = nullptr;
    if (fopen_s(&f, cfg_.infile, "r") != 0 || !f) return -1;

    // Buffer for reading lines; large but stack-allocated to avoid heap during processing.
    char line[512];
    Order o;
    Execution out_execs[64];

    while (std::fgets(line, sizeof(line), f)) {
        if (!parse_line(line, o)) continue;

        // Per-tick measurement (optional)
        nanomarket::latency::ScopedTimer t("replay_tick");

        // Risk check
        bool ok = risk_.check_new_order(o.price, o.qty, o.side);
        if (!ok) {
            std::fprintf(logf_, "TS=%lld,ORDER=%llu,%c,REJECTED\n", (long long)o.ts, (unsigned long long)o.id, (o.side==Side::Buy)?'B':'S');
            continue;
        }

        // Submit to order book using caller-provided buffer (no heap)
        size_t n = book_.submit_order(o, out_execs, 64);
        for (size_t i = 0; i < n; ++i) {
            auto &e = out_execs[i];
            risk_.on_fill(e.price, e.filled_qty, (o.side==Side::Buy)?Side::Buy:Side::Sell);
            std::fprintf(logf_, "TS=%lld,EXEC,rest=%llu,in=%llu,qty=%d,px=%lld\n", (long long)e.ts, (unsigned long long)e.resting_id, (unsigned long long)e.incoming_id, (int)e.filled_qty, (long long)e.price);
        }

        // Log deterministic risk snapshot after processing this tick using atomic readers.
        // Acquire ordering in readers ensures a consistent view of incremental updates
        // without blocking or locks. These reads are cheap and safe for single-threaded
        // replay; they are also safe to call from other threads for monitoring.
        int64_t pos = risk_.position();
        int64_t notl = risk_.notional();
        std::fprintf(logf_, "TS=%lld,RISK,position=%lld,notional=%lld\n", (long long)o.ts, (long long)pos, (long long)notl);
    }

    std::fclose(f);
    return 0;
}
