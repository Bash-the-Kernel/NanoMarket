// exchange/order.hpp
#pragma once

#include "core/types.hpp"

namespace nanomarket::exchange {

struct Order {
    core::OrderId id{0};
    core::Price price{0};
    core::Qty qty{0};
    core::Qty remaining{0};
    core::Timestamp ts{0};
    core::Side side{core::Side::Buy};
    // index of next order in linked list for a price level; -1 if none
    int32_t next{-1};
};

struct Execution {
    core::OrderId resting_id;
    core::OrderId incoming_id;
    core::Qty filled_qty;
    core::Price price;
    core::Timestamp ts;
};

} // namespace nanomarket::exchange
