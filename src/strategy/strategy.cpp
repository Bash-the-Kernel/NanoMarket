#include "strategy/strategy.hpp"
#include "core/types.hpp"

#include <chrono>
#include <iostream>

using namespace nanomarket::core;
namespace ms = nanomarket::strategy;

ms::MarketMaker::MarketMaker(nanomarket::utils::SpscRing<nanomarket::exchange::Order, 1024>* out_ring)
    : out_ring_(out_ring) {}

ms::MarketMaker::~MarketMaker() { stop(); }

void ms::MarketMaker::start() {
    running_.store(true, std::memory_order_release);
    thr_ = std::thread(&MarketMaker::run, this);
}

void ms::MarketMaker::stop() {
    running_.store(false, std::memory_order_release);
    if (thr_.joinable()) thr_.join();
}

void ms::MarketMaker::run() {
    // Deterministic pseudo-market making loop: place symmetric quotes around ref.
    core::Price ref_price = 10000;
    core::OrderId id_counter = 1;
    while (running_.load(std::memory_order_acquire)) {
        int spread = params_.spread_ticks.load(std::memory_order_relaxed);
        int size = params_.size.load(std::memory_order_relaxed);

        // place bid
        nanomarket::exchange::Order bid;
        bid.id = id_counter++;
        bid.side = Side::Buy;
        bid.price = ref_price - spread;
        bid.qty = static_cast<Qty>(size);
        bid.ts = now_ns();
        out_ring_->push(bid);

        // place ask
        nanomarket::exchange::Order ask = bid;
        ask.side = Side::Sell;
        ask.id = id_counter++;
        ask.price = ref_price + spread;
        ask.ts = now_ns();
        out_ring_->push(ask);

        // sleep a deterministic interval (simple throttle)
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}
