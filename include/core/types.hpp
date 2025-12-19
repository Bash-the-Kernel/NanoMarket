// core/types.hpp
#pragma once

#include <cstdint>
#include <chrono>

namespace nanomarket::core {

using Timestamp = std::int64_t; // nanoseconds since epoch or start of run
using OrderId = std::uint64_t;
using Price = std::int64_t; // integer ticks
using Qty = std::int32_t;

enum class Side : int8_t { Buy = 1, Sell = -1 };

inline Timestamp now_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

} // namespace nanomarket::core
