#include "risk/risk.hpp"
#include <cassert>
#include <iostream>

using namespace nanomarket::risk;
using namespace nanomarket::core;

int main() {
    RiskEngine r;
    r.limits().max_position = 5;
    r.limits().max_order_size = 3;
    r.limits().max_notional = 1000000;

    // order exceeding max_order_size rejected
    bool ok1 = r.check_new_order(10000, 10, Side::Buy);
    if (ok1) { std::cerr << "expected order > max_order_size to be rejected\n"; return 1; }

    // multiple small fills update position; enforce limit
    bool ok2 = r.check_new_order(10000, 3, Side::Buy);
    if (!ok2) { std::cerr << "expected small order to be accepted\n"; return 1; }
    r.on_fill(10000, 3, Side::Buy);

    bool ok3 = r.check_new_order(10000, 3, Side::Buy);
    // this would push position to 6 > 5 -> should be rejected
    if (ok3) { std::cerr << "expected order pushing position over limit to be rejected\n"; return 1; }

    std::cout << "test_risk: PASS\n";
    return 0;
}
