// strategy/strategy.hpp
#pragma once

#include "core/types.hpp"
#include "exchange/order.hpp"
#include "utils/ring_buffer.hpp"

#include <atomic>
#include <thread>

namespace nanomarket::strategy {

struct Params {
    std::atomic<int32_t> spread_ticks{2};
    std::atomic<int32_t> size{1};
    std::atomic<int32_t> max_size{10};
};

class MarketMaker {
public:
    MarketMaker(nanomarket::utils::SpscRing<nanomarket::exchange::Order, 1024>* out_ring);
    ~MarketMaker();

    void start();
    void stop();
    Params& params() noexcept { return params_; }

private:
    void run();
    std::thread thr_;
    std::atomic<bool> running_{false};
    Params params_;
    nanomarket::utils::SpscRing<nanomarket::exchange::Order, 1024>* out_ring_;
};

} // namespace nanomarket::strategy
