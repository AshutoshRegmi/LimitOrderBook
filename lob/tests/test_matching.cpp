#include "MatchingEngine.h"
#include "OrderBook.h"

#include <cassert>
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

void test_simple_cross() {
  OrderBook book;
  MatchingEngine engine(book);

  book.add({1, Side::SELL, OrderType::LIMIT, 10000, 50, "AAPL", 1});
  auto trades = engine.process(
      {2, Side::BUY, OrderType::LIMIT, 10000, 50, "AAPL", 2});

  ASSERT(trades.size() == 1);
  ASSERT(trades[0].quantity == 50);
  ASSERT(trades[0].price == 10000);
  ASSERT(!book.topAsk().has_value());
  ASSERT(!book.topBid().has_value());
}

void test_partial_fill() {
  OrderBook book;
  MatchingEngine engine(book);

  book.add({1, Side::SELL, OrderType::LIMIT, 10000, 60, "AAPL", 1});
  auto trades = engine.process(
      {2, Side::BUY, OrderType::LIMIT, 10000, 100, "AAPL", 2});

  ASSERT(trades.size() == 1);
  ASSERT(trades[0].quantity == 60);
  ASSERT(book.topBid().has_value());
  ASSERT(book.topBid()->id == 2);
  ASSERT(book.topBid()->quantity == 40);
}

void test_walk_the_book() {
  OrderBook book;
  MatchingEngine engine(book);

  book.add({1, Side::SELL, OrderType::LIMIT, 10000, 50, "AAPL", 1});
  book.add({2, Side::SELL, OrderType::LIMIT, 10100, 50, "AAPL", 2});
  auto trades = engine.process(
      {3, Side::BUY, OrderType::LIMIT, 10200, 80, "AAPL", 3});

  ASSERT(trades.size() == 2);
  ASSERT(trades[0].quantity == 50 && trades[0].price == 10000);
  ASSERT(trades[1].quantity == 30 && trades[1].price == 10100);
  ASSERT(!book.topBid().has_value());
  ASSERT(book.topAsk()->quantity == 20);
}

void test_no_match() {
  OrderBook book;
  MatchingEngine engine(book);

  book.add({1, Side::SELL, OrderType::LIMIT, 10100, 50, "AAPL", 1});
  auto trades = engine.process(
      {2, Side::BUY, OrderType::LIMIT, 10000, 50, "AAPL", 2});

  ASSERT(trades.empty());
  ASSERT(book.topBid()->id == 2);
  ASSERT(book.topAsk()->id == 1);
}

int main() {
  test_simple_cross();
  test_partial_fill();
  test_walk_the_book();
  test_no_match();

  std::cout << tests_passed << "/" << tests_run << " tests passed\n";
  return tests_passed == tests_run ? 0 : 1;
}
