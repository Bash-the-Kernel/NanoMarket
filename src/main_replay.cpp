#include "exchange/market_replayer.hpp"
#include "exchange/order_book.hpp"
#include "risk/risk.hpp"

#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: main_replay <input_csv> [output_log]\n";
        return 1;
    }

    const char* infile = argv[1];
    const char* outfile = (argc >= 3) ? argv[2] : "replay.log";

    nanomarket::exchange::OrderBook::Config cfg{1, 64, 1024, 10000};
    nanomarket::exchange::OrderBook book(cfg);
    nanomarket::risk::RiskEngine risk;

    nanomarket::exchange::MarketDataReplayer::Config rcfg;
    rcfg.infile = infile;
    rcfg.outfile = outfile;

    nanomarket::exchange::MarketDataReplayer replayer(rcfg, book, risk);
    int r = replayer.run();
    if (r != 0) {
        std::cerr << "Replay failed\n";
        return r;
    }
    std::cout << "Replay complete. Output: " << outfile << "\n";
    return 0;
}
