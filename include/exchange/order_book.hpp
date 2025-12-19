// exchange/order_book.hpp
#pragma once

#include "core/types.hpp"
#include "exchange/order.hpp"

#include <array>
#include <vector>
#include <optional>
#include <cstdint>

namespace nanomarket::exchange {

// Fixed-size, deterministic order book. No heap allocation in matching hot path.
// Trade-offs: price ladder is a simple fixed array of price levels centered on a reference price.
// Complexity: insertion/search is O(L) where L is number of levels; L is small (configurable).

class OrderBook {
public:
    struct Config { core::Price tick; int32_t levels; int32_t max_orders; core::Price ref_price; };

    explicit OrderBook(const Config& cfg) noexcept;

    // Submit order into book; matching occurs immediately (single-threaded). Returns executions.
    std::vector<Execution> submit_order(const Order& o) noexcept;

    // Cancel order by id (best-effort)
    bool cancel(core::OrderId id) noexcept;

private:
    const Config cfg_;

    // preallocated pool
    std::vector<Order> pool_;
    int32_t free_head_; // index into pool_

    // price levels represented as heads of singly linked lists (indices into pool_)
    std::vector<int32_t> bids_; // index 0 best_bid
    std::vector<int32_t> asks_; // index 0 best_ask

    // helper methods
    int32_t alloc_order(const Order& o) noexcept;
    void free_order(int32_t idx) noexcept;
    int32_t price_to_level(core::Price p) const noexcept;
};

} // namespace nanomarket::exchange
