#include "exchange/order_book.hpp"
#include <cassert>
#include <iostream>

using namespace nanomarket::exchange;
using namespace nanomarket::core;

int main() {
    OrderBook::Config cfg{1, 8, 128, 10000};
    OrderBook book(cfg);

    // Create deterministic limit orders: A sells at price 10002 qty 5, then B buys at 10002 qty 3
    Order o1; o1.id = 1; o1.price = 10002; o1.qty = 5; o1.remaining = 5; o1.side = Side::Sell; o1.ts = 1;
    Order o2; o2.id = 2; o2.price = 10002; o2.qty = 3; o2.remaining = 3; o2.side = Side::Buy; o2.ts = 2;

    // Insert resting sell
    nanomarket::exchange::Execution out[16];
    size_t n1 = book.submit_order(o1, out, 16);
    if (n1 != 0) { std::cerr << "expected 0 executions for o1, got " << n1 << "\n"; return 1; }

    // Incoming buy should partially fill
    size_t n2 = book.submit_order(o2, out, 16);
    if (n2 != 1) { std::cerr << "expected 1 execution for o2, got " << n2 << "\n"; return 1; }
    if (out[0].filled_qty != 3) { std::cerr << "expected filled 3 got " << out[0].filled_qty << "\n"; return 1; }
    // Remaining resting should be 2 (5-3)

    // Re-submit a buy to take remaining
    Order o3; o3.id = 3; o3.price = 10002; o3.qty = 2; o3.remaining = 2; o3.side = Side::Buy; o3.ts = 3;
    size_t n3 = book.submit_order(o3, out, 16);
    if (n3 != 1) { std::cerr << "expected 1 execution for o3, got " << n3 << "\n"; return 1; }
    if (out[0].filled_qty != 2) { std::cerr << "expected filled 2 got " << out[0].filled_qty << "\n"; return 1; }

    // Determinism check: re-run the same sequence on a fresh book and compare executions
    OrderBook book2(cfg);
    nanomarket::exchange::Execution out2[16];
    book2.submit_order(o1, out2, 16);
    size_t e2_1 = book2.submit_order(o2, out2, 16);
    size_t e2_2 = book2.submit_order(o3, out2, 16);
    if (e2_1 != n2) { std::cerr << "determinism failure: e2_1 != n2\n"; return 1; }
    if (e2_2 != n3) { std::cerr << "determinism failure: e2_2 != n3\n"; return 1; }

    std::cout << "test_order_book: PASS\n";
    return 0;
}
