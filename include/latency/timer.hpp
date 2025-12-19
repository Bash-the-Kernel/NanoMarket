// RAII nanosecond timer with minimal overhead
#pragma once

#include <cstdint>
#include <string>

namespace nanomarket::latency {

struct SamplePoint {
    const char* label;
    int64_t start_ns;
    int64_t end_ns;
};

class ScopedTimer {
public:
    explicit ScopedTimer(const char* label) noexcept;
    ~ScopedTimer() noexcept;

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;

private:
    const char* label_;
    int64_t start_;
};

// Reporting API (simple file sink implemented in .cpp)
void report_sample(const SamplePoint& s) noexcept;

} // namespace nanomarket::latency
