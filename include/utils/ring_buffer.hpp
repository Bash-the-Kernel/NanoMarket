// Single-producer single-consumer lock-free ring buffer
#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>

namespace nanomarket::utils {

template<typename T, size_t N>
class SpscRing {
    static_assert((N & (N-1)) == 0, "N must be power of two");
public:
    SpscRing() noexcept : head_(0), tail_(0) {}

    bool push(const T& item) noexcept {
        const auto head = head_.load(std::memory_order_relaxed);
        const auto next = (head + 1) & mask_;
        if (next == tail_.load(std::memory_order_acquire)) return false; // full
        buffer_[head] = item;
        head_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& out) noexcept {
        const auto tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire)) return false; // empty
        out = buffer_[tail];
        tail_.store((tail + 1) & mask_, std::memory_order_release);
        return true;
    }

    bool empty() const noexcept { return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire); }

private:
    static constexpr size_t mask_ = N - 1;
    T buffer_[N];
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

} // namespace nanomarket::utils
