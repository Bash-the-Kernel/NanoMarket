#include "exchange/market_replayer.hpp"
#include "exchange/order_book.hpp"
#include "risk/risk.hpp"

#include <fstream>
#include <string>
#include <iostream>

int main() {
    using namespace nanomarket::exchange;
    using namespace nanomarket::core;

    // Paths: input in source tree, output in build tree (relative paths used by CTest)
    const char* infile = "..\\data\\sample_replay.csv"; // CTest runs from build dir
    const char* outfile = "replay_test_output.log";
    const char* golden = "..\\tests\\golden_replay.log";

    OrderBook::Config cfg{1, 64, 1024, 10000};
    OrderBook book(cfg);
    nanomarket::risk::RiskEngine risk;

    MarketDataReplayer::Config rcfg;
    rcfg.infile = infile;
    rcfg.outfile = outfile;

    MarketDataReplayer replayer(rcfg, book, risk);
    int r = replayer.run();
    if (r != 0) {
        std::cerr << "Replay run failed with code " << r << "\n";
        return 1;
    }

    // Compare output line-by-line with golden file
    std::ifstream outfs(outfile);
    std::ifstream gfs(golden);
    if (!outfs.is_open()) { std::cerr << "Cannot open output file\n"; return 1; }
    if (!gfs.is_open()) { std::cerr << "Cannot open golden file\n"; return 1; }

    std::string outline, gline;
    size_t lineno = 1;
    while (true) {
        bool out_ok = static_cast<bool>(std::getline(outfs, outline));
        bool g_ok = static_cast<bool>(std::getline(gfs, gline));
        if (!out_ok && !g_ok) break; // both finished
        if (out_ok != g_ok) {
            std::cerr << "Line count mismatch at line " << lineno << "\n";
            return 1;
        }
        if (outline != gline) {
            std::cerr << "Mismatch at line " << lineno << "\n";
            std::cerr << "Expected: '" << gline << "'\n";
            std::cerr << "Got     : '" << outline << "'\n";
            return 1;
        }
        ++lineno;
    }

    std::cout << "replay_test: PASS\n";
    return 0;
}
