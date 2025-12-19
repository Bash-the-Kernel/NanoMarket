#include "exchange/order_book.hpp"
#include "latency/timer.hpp"
#include "strategy/strategy.hpp"
#include "risk/risk.hpp"
#include "utils/ring_buffer.hpp"

#include <iostream>
#include <thread>
#include <vector>

using namespace nanomarket::core;

int main() {
    // Build components
    nanomarket::exchange::OrderBook::Config cfg{1, 64, 1024, 10000};
    nanomarket::exchange::OrderBook book(cfg);

    nanomarket::utils::SpscRing<nanomarket::exchange::Order, 1024> ring;
    nanomarket::strategy::MarketMaker mm(&ring);
    nanomarket::risk::RiskEngine risk;

    mm.start();

    // main loop: consume orders from strategy and pass to exchange through risk
    for (int iter = 0; iter < 1000; ++iter) {
        nanomarket::exchange::Order o;
        while (ring.pop(o)) {
            // latency measurement: measure tick-to-trade start
            {
                nanomarket::latency::ScopedTimer t("tick_to_submit");
                if (!risk.check_new_order(o.price, o.qty, o.side)) {
                    std::cout << "Order rejected by risk id=" << o.id << "\n";
                    continue;
                }
                auto execs = book.submit_order(o);
                for (auto &e : execs) {
                    risk.on_fill(e.price, e.filled_qty, (o.side == Side::Buy) ? Side::Buy : Side::Sell);
                    std::cout << "Exec: resting=" << e.resting_id << " incoming=" << e.incoming_id << " qty=" << e.filled_qty << "@" << e.price << "\n";
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }

    mm.stop();
    std::cout << "Run complete." << std::endl;
    return 0;
}
