#include "Benchmark.h"
#include "Exchange.h"
#include "OrderBook.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  if (argc > 1 && std::string(argv[1]) == "--benchmark") {
    runBenchmark();
    return 0;
  }

  OrderBook book;

  std::cout << "=== Demo: Cancel + modify ===\n";

  book.add({1, Side::BUY, OrderType::LIMIT, 10050, 100, "AAPL", 1});
  book.add({2, Side::BUY, OrderType::LIMIT, 10050, 200, "AAPL", 2});
  book.add({3, Side::BUY, OrderType::LIMIT, 10050, 300, "AAPL", 3});
  book.add({4, Side::SELL, OrderType::LIMIT, 10100, 100, "AAPL", 4});
  book.add({5, Side::SELL, OrderType::LIMIT, 10125, 200, "AAPL", 5});

  book.print();

  std::cout << "=== Cancel order 2 (middle of $100.50 level) ===\n";
  book.cancel(2);
  book.print();

  std::cout << "=== Modify order 3: 300 -> 50 shares ===\n";
  book.modify(3, 50);
  book.print();

  std::cout << "=== Modify order 1: move $100.50 -> $100.75 ===\n";
  book.modify(1, 100, 10075);
  book.print();

  std::cout << "=== Cancel filled order 999 (should fail) ===\n";
  if (!book.cancel(999)) {
    std::cout << "Correctly rejected cancel for unknown order.\n";
  }

  std::cout << "\n=== Multi-symbol: AAPL and GOOG are isolated ===\n";
  Exchange exchange;
  exchange.setVerbose(true);
  exchange.process({10, Side::SELL, OrderType::LIMIT, 10000, 50, "AAPL", 10});
  exchange.process({11, Side::SELL, OrderType::LIMIT, 50000, 50, "GOOG", 11});
  exchange.process({12, Side::BUY, OrderType::LIMIT, 10000, 50, "AAPL", 12});
  std::cout << "GOOG sell still resting: "
            << (exchange.getOrder("GOOG", 11).has_value() ? "yes" : "no") << '\n';
  std::cout << "Active symbols: " << exchange.symbolCount() << '\n';

  return 0;
}
