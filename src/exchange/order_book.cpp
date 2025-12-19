#include "exchange/order_book.hpp"
#include "core/types.hpp"

#include <algorithm>
#include <cassert>

using namespace nanomarket::core;
namespace em = nanomarket::exchange;

em::OrderBook::OrderBook(const Config& cfg) noexcept
    : cfg_(cfg), pool_(), free_head_(-1), bids_(), asks_() {
    pool_.resize(cfg.max_orders);
    // initialize free list
    for (int32_t i = 0; i < cfg.max_orders; ++i) {
        pool_[i].next = i + 1;
    }
    pool_[cfg.max_orders - 1].next = -1;
    free_head_ = 0;

    bids_.assign(cfg.levels, -1);
    asks_.assign(cfg.levels, -1);
}

int32_t em::OrderBook::alloc_order(const Order& o) noexcept {
    if (free_head_ == -1) return -1;
    int32_t idx = free_head_;
    free_head_ = pool_[idx].next;
    pool_[idx] = o;
    pool_[idx].remaining = o.qty;
    pool_[idx].next = -1;
    return idx;
}

void em::OrderBook::free_order(int32_t idx) noexcept {
    pool_[idx].next = free_head_;
    free_head_ = idx;
}

int32_t em::OrderBook::price_to_level(core::Price p) const noexcept {
    // levels centered on ref_price; positive levels for better prices
    const auto diff = p - cfg_.ref_price;
    const int64_t step = cfg_.tick;
    int64_t lvl = diff / step + cfg_.levels / 2;
    if (lvl < 0) lvl = 0;
    if (lvl >= cfg_.levels) lvl = cfg_.levels - 1;
    return static_cast<int32_t>(lvl);
}

size_t em::OrderBook::submit_order(const Order& o, Execution* out, size_t max_out) noexcept {
    // Single-threaded deterministic matching loop; write executions into caller buffer
    size_t produced = 0;

    bool is_market = (o.price == 0);
    int incoming_remaining = o.qty;

    if (o.side == Side::Buy) {
        // match against asks (best first)
        for (int lvl = 0; lvl < static_cast<int>(asks_.size()) && incoming_remaining > 0; ++lvl) {
            int32_t cur = asks_[lvl];
            int32_t prev = -1;
            while (cur != -1 && incoming_remaining > 0) {
                Order& r = pool_[cur];
                int filled = std::min<int>(incoming_remaining, r.remaining);
                // Use deterministic execution timestamp: internal sequential counter.
                if (produced < max_out) out[produced] = Execution{r.id, o.id, static_cast<Qty>(filled), r.price, exec_seq_++};
                ++produced;
                r.remaining -= filled;
                incoming_remaining -= filled;
                if (r.remaining == 0) {
                    int32_t next = r.next;
                    if (prev == -1) {
                        asks_[lvl] = next;
                    } else {
                        pool_[prev].next = next;
                    }
                    free_order(cur);
                    cur = next;
                } else {
                    prev = cur;
                    cur = r.next;
                }
            }
        }
    } else {
        // incoming sell matches bids
        for (int lvl = 0; lvl < static_cast<int>(bids_.size()) && incoming_remaining > 0; ++lvl) {
            int32_t cur = bids_[lvl];
            int32_t prev = -1;
            while (cur != -1 && incoming_remaining > 0) {
                Order& r = pool_[cur];
                int filled = std::min<int>(incoming_remaining, r.remaining);
                // Use deterministic execution timestamp: internal sequential counter.
                if (produced < max_out) out[produced] = Execution{r.id, o.id, static_cast<Qty>(filled), r.price, exec_seq_++};
                ++produced;
                r.remaining -= filled;
                incoming_remaining -= filled;
                if (r.remaining == 0) {
                    int32_t next = r.next;
                    if (prev == -1) {
                        bids_[lvl] = next;
                    } else {
                        pool_[prev].next = next;
                    }
                    free_order(cur);
                    cur = next;
                } else {
                    prev = cur;
                    cur = r.next;
                }
            }
        }
    }

    // If residual remains and incoming was a limit order, insert resting order at price level (FIFO)
    if (!is_market && incoming_remaining > 0) {
        Order resting = o;
        resting.qty = incoming_remaining;
        resting.remaining = incoming_remaining;
        // Preserve incoming deterministic timestamp instead of using system clock
        resting.ts = o.ts;
        int32_t idx = alloc_order(resting);
        if (idx >= 0) {
            int lvl = price_to_level(resting.price);
            int32_t head = (o.side == Side::Buy) ? bids_[lvl] : asks_[lvl];
            if (head == -1) {
                if (o.side == Side::Buy) bids_[lvl] = idx; else asks_[lvl] = idx;
            } else {
                // append to tail for FIFO
                int32_t cur = head;
                while (pool_[cur].next != -1) cur = pool_[cur].next;
                pool_[cur].next = idx;
            }
        }
    }

    return produced;
}

bool em::OrderBook::cancel(core::OrderId id) noexcept {
    // best-effort linear scan (deterministic). O(L+M)
    for (auto& levels : {&bids_, &asks_}) {
        for (size_t lvl = 0; lvl < levels->size(); ++lvl) {
            int32_t cur = (*levels)[lvl];
            int32_t prev = -1;
            while (cur != -1) {
                if (pool_[cur].id == id) {
                    int32_t next = pool_[cur].next;
                    if (prev == -1) (*levels)[lvl] = next; else pool_[prev].next = next;
                    free_order(cur);
                    return true;
                }
                prev = cur;
                cur = pool_[cur].next;
            }
        }
    }
    return false;
}
