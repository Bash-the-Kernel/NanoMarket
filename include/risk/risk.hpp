// risk/risk.hpp
#pragma once

#include "core/types.hpp"

#include <atomic>

namespace nanomarket::risk {

struct Limits {
    std::atomic<int64_t> max_position{100};
    std::atomic<int64_t> max_order_size{50};
    std::atomic<int64_t> max_notional{1000000};
};

class RiskEngine {
public:
    RiskEngine() noexcept;

    bool check_new_order(core::Price price, core::Qty qty, core::Side side) noexcept;
    void on_fill(core::Price price, core::Qty qty, core::Side side) noexcept;

    Limits& limits() noexcept { return limits_; }

    // Non-blocking atomic readers for deterministic snapshots.
    // These return the current single-instrument position and notional exposure.
    // Use acquire ordering to ensure a consistent view when used from other threads.
    int64_t position() const noexcept { return position_.load(std::memory_order_acquire); }
    int64_t notional() const noexcept { return notional_.load(std::memory_order_acquire); }

private:
    std::atomic<int64_t> position_{0}; // simplified single-instrument position
    std::atomic<int64_t> notional_{0};
    Limits limits_;
};

} // namespace nanomarket::risk
