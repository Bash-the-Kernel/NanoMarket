#include "latency/timer.hpp"
#include "core/types.hpp"

#include <fstream>
#include <atomic>

namespace nanomarket::latency {

static std::ofstream& get_log() {
    static std::ofstream ofs("latency_samples.log", std::ios::app);
    return ofs;
}

void report_sample(const SamplePoint& s) noexcept {
    auto& ofs = get_log();
    // minimal formatting; keeping it simple so hot paths only call inline functions
    ofs << s.label << ',' << s.start_ns << ',' << s.end_ns << '\n';
}

ScopedTimer::ScopedTimer(const char* label) noexcept : label_(label), start_(core::now_ns()) {}

ScopedTimer::~ScopedTimer() noexcept {
    SamplePoint s{label_, start_, core::now_ns()};
    report_sample(s);
}

} // namespace nanomarket::latency
