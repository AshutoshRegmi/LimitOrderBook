#include "Exchange.h"

#include <iostream>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT(cond)                                                          \
  do {                                                                        \
    ++tests_run;                                                              \
    if (cond) {                                                               \
      ++tests_passed;                                                         \
    } else {                                                                  \
      std::cerr << "FAIL: " << #cond << " (" << __FILE__ << ":" << __LINE__  \
                << ")\n";                                                     \
    }                                                                         \
  } while (0)

void test_symbols_do_not_cross_match() {
  Exchange exchange;

  exchange.process({1, Side::SELL, OrderType::LIMIT, 10000, 50, "AAPL", 1});
  const auto trades = exchange.process(
      {2, Side::BUY, OrderType::LIMIT, 10000, 50, "GOOG", 2});

  ASSERT(trades.empty());
  ASSERT(exchange.getOrder("AAPL", 1).has_value());
  ASSERT(exchange.getOrder("GOOG", 2).has_value());
  ASSERT(exchange.symbolCount() == 2);
}

void test_same_symbol_still_matches() {
  Exchange exchange;

  exchange.process({1, Side::SELL, OrderType::LIMIT, 10000, 50, "AAPL", 1});
  const auto trades = exchange.process(
      {2, Side::BUY, OrderType::LIMIT, 10000, 50, "AAPL", 2});

  ASSERT(trades.size() == 1);
  ASSERT(trades[0].quantity == 50);
  ASSERT(!exchange.getOrder("AAPL", 1).has_value());
  ASSERT(!exchange.getOrder("AAPL", 2).has_value());
}

void test_cancel_routed_by_symbol() {
  Exchange exchange;

  exchange.process({1, Side::BUY, OrderType::LIMIT, 10000, 100, "AAPL", 1});
  exchange.process({2, Side::BUY, OrderType::LIMIT, 20000, 100, "GOOG", 2});

  ASSERT(exchange.cancel("AAPL", 1));
  ASSERT(!exchange.getOrder("AAPL", 1).has_value());
  ASSERT(exchange.getOrder("GOOG", 2).has_value());
  ASSERT(!exchange.cancel("AAPL", 1));
}

void test_modify_routed_by_symbol() {
  Exchange exchange;

  exchange.process({1, Side::BUY, OrderType::LIMIT, 10000, 100, "AAPL", 1});
  exchange.process({2, Side::BUY, OrderType::LIMIT, 20000, 100, "GOOG", 2});

  ASSERT(exchange.modify("GOOG", 2, 40));
  ASSERT(exchange.getOrder("GOOG", 2)->quantity == 40);
  ASSERT(exchange.getOrder("AAPL", 1)->quantity == 100);
}

void test_lazy_book_creation() {
  Exchange exchange;

  ASSERT(exchange.symbolCount() == 0);
  exchange.process({1, Side::BUY, OrderType::LIMIT, 10000, 10, "MSFT", 1});
  ASSERT(exchange.symbolCount() == 1);
  exchange.process({2, Side::BUY, OrderType::LIMIT, 50000, 10, "TSLA", 2});
  ASSERT(exchange.symbolCount() == 2);
  ASSERT(exchange.book("MSFT") != nullptr);
  ASSERT(exchange.book("TSLA") != nullptr);
  ASSERT(exchange.book("AAPL") == nullptr);
}

int main() {
  test_symbols_do_not_cross_match();
  test_same_symbol_still_matches();
  test_cancel_routed_by_symbol();
  test_modify_routed_by_symbol();
  test_lazy_book_creation();

  std::cout << tests_passed << "/" << tests_run << " tests passed\n";
  return tests_passed == tests_run ? 0 : 1;
}
