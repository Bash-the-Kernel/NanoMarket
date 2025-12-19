#include "risk/risk.hpp"
#include "core/types.hpp"

using namespace nanomarket::core;
namespace r = nanomarket::risk;

r::RiskEngine::RiskEngine() noexcept {
}

bool r::RiskEngine::check_new_order(core::Price price, core::Qty qty, core::Side side) noexcept {
    // quick read-mostly checks using relaxed atomics where safe
    if (std::llabs(qty) > limits_.max_order_size.load(std::memory_order_relaxed)) return false;
    int64_t pos = position_.load(std::memory_order_relaxed);
    int64_t newpos = pos + static_cast<int64_t>(static_cast<int>(side) * qty);
    if (std::llabs(newpos) > limits_.max_position.load(std::memory_order_relaxed)) return false;
    int64_t notional = notional_.load(std::memory_order_relaxed);
    int64_t newnot = notional + static_cast<int64_t>(price) * qty;
    if (std::llabs(newnot) > limits_.max_notional.load(std::memory_order_relaxed)) return false;
    return true;
}

void r::RiskEngine::on_fill(core::Price price, core::Qty qty, core::Side side) noexcept {
    // incremental updates
    int64_t delta = static_cast<int64_t>(static_cast<int>(side) * qty);
    position_.fetch_add(delta, std::memory_order_relaxed);
    notional_.fetch_add(static_cast<int64_t>(price) * qty, std::memory_order_relaxed);
}
