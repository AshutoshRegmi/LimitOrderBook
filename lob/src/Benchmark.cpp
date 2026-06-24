#include "Benchmark.h"

#include "Exchange.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int kNumOrders = 1'000'000;
constexpr int kWarmupOps = 10'000;
constexpr Price kBasePrice = 10000;
constexpr Price kPriceSpread = 500;
constexpr int kCancelPct = 20;

const char* kSymbols[] = {"AAPL", "GOOG", "MSFT", "AMZN", "TSLA"};
constexpr int kSymbolCount = 5;

enum class OpKind { Add, Cancel };

struct LiveOrder {
    std::string symbol;
    OrderId id;
};

struct BenchOp {
    OpKind kind;
    Order order;
};

struct BenchStats {
    int adds = 0;
    int cancels = 0;
    int cancel_skipped = 0;
    int trades = 0;
    int adds_with_trades = 0;
};

std::vector<BenchOp> generateWorkload(std::mt19937& rng) {
    std::vector<BenchOp> ops;
    ops.reserve(kNumOrders);

    std::uniform_int_distribution<Price> price_dist(kBasePrice - kPriceSpread,
                                                    kBasePrice + kPriceSpread);
    std::uniform_int_distribution<Quantity> qty_dist(1, 1000);
    std::uniform_int_distribution<int> side_dist(0, 1);
    std::uniform_int_distribution<int> kind_dist(0, 99);
    std::uniform_int_distribution<int> symbol_dist(0, kSymbolCount - 1);

    OrderId next_id = 1;
    for (int i = 0; i < kNumOrders; ++i) {
        if (kind_dist(rng) < kCancelPct) {
            ops.push_back({OpKind::Cancel, {}});
        } else {
            const Side side = side_dist(rng) == 0 ? Side::BUY : Side::SELL;
            ops.push_back({OpKind::Add,
                           {next_id++,
                            side,
                            OrderType::LIMIT,
                            price_dist(rng),
                            qty_dist(rng),
                            kSymbols[symbol_dist(rng)],
                            static_cast<uint64_t>(i)}});
        }
    }
    return ops;
}

BenchStats runOps(Exchange& exchange, const std::vector<BenchOp>& ops,
                  std::mt19937& cancel_rng) {
    BenchStats stats;
    std::vector<LiveOrder> live;

    for (const auto& op : ops) {
        if (op.kind == OpKind::Add) {
            const auto trades = exchange.process(op.order);
            ++stats.adds;
            if (!trades.empty()) {
                ++stats.adds_with_trades;
                stats.trades += static_cast<int>(trades.size());
            }
            if (exchange.getOrder(op.order.symbol, op.order.id).has_value()) {
                live.push_back({op.order.symbol, op.order.id});
            }
        } else {
            bool cancelled = false;
            while (!live.empty()) {
                std::uniform_int_distribution<size_t> dist(0, live.size() - 1);
                const size_t idx = dist(cancel_rng);
                const LiveOrder target = live[idx];
                live[idx] = live.back();
                live.pop_back();
                if (!exchange.getOrder(target.symbol, target.id).has_value()) {
                    continue;
                }
                if (exchange.cancel(target.symbol, target.id)) {
                    ++stats.cancels;
                    cancelled = true;
                }
                break;
            }
            if (!cancelled) {
                ++stats.cancel_skipped;
            }
        }
    }
    return stats;
}

void printAndSaveResults(const BenchStats& stats, int64_t elapsed_ns) {
    const double orders_per_sec =
        static_cast<double>(kNumOrders) / (static_cast<double>(elapsed_ns) / 1e9);
    const double avg_latency_ns = static_cast<double>(elapsed_ns) / kNumOrders;
    const int attempted = stats.adds + stats.cancels + stats.cancel_skipped;
    const double add_pct = 100.0 * stats.adds / attempted;
    const double cancel_pct = 100.0 * stats.cancels / attempted;
    const double match_pct = 100.0 * stats.adds_with_trades / stats.adds;

    std::ostringstream report;
    report << std::fixed << std::setprecision(1);
    report << "=== Limit Order Book Benchmark ===\n";
    report << "Symbols: " << kSymbolCount << " (AAPL, GOOG, MSFT, AMZN, TSLA)\n";
    report << "Orders processed: " << kNumOrders << '\n';
    report << "Throughput: " << static_cast<int>(orders_per_sec) << " orders/sec\n";
    report << "Avg latency: " << avg_latency_ns << " ns/order ("
           << (avg_latency_ns / 1000.0) << " us/order)\n";
    report << "Workload: " << add_pct << "% adds, " << cancel_pct << "% cancels, "
           << match_pct << "% of adds produced trades\n";
    report << "Trades executed: " << stats.trades << '\n';
    report << "Cancels skipped (no live orders): " << stats.cancel_skipped << '\n';

    const std::string output = report.str();
    std::cout << output;

    std::ofstream out("benchmarks/results.txt");
    if (out) {
        out << output;
        std::cout << "\nResults saved to benchmarks/results.txt\n";
    } else {
        std::cerr << "Warning: could not write benchmarks/results.txt\n";
    }
}

}  // namespace

void runBenchmark() {
    std::mt19937 rng(42);
    std::mt19937 cancel_rng(99);
    const auto ops = generateWorkload(rng);

    {
        Exchange warmup;
        warmup.setVerbose(false);
        const int warmup_count = std::min(kWarmupOps, static_cast<int>(ops.size()));
        runOps(warmup,
               std::vector<BenchOp>(ops.begin(), ops.begin() + warmup_count), cancel_rng);
    }

    Exchange exchange;
    exchange.setVerbose(false);

    const auto start = std::chrono::high_resolution_clock::now();
    const auto stats = runOps(exchange, ops, cancel_rng);
    const auto end = std::chrono::high_resolution_clock::now();

    const auto elapsed_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    printAndSaveResults(stats, elapsed_ns);
}
