#include "OrderBook.h"

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

void test_cancel_front_of_level() {
  OrderBook book;

  book.add({1, Side::BUY, OrderType::LIMIT, 10000, 100, "AAPL", 1});
  book.add({2, Side::BUY, OrderType::LIMIT, 10000, 200, "AAPL", 2});
  book.add({3, Side::BUY, OrderType::LIMIT, 10000, 300, "AAPL", 3});

  ASSERT(book.cancel(1));
  ASSERT(book.getOrder(1) == std::nullopt);
  ASSERT(book.getOrder(2).has_value());
  ASSERT(book.topBid()->id == 2);
}

void test_cancel_middle_of_level() {
  OrderBook book;

  book.add({1, Side::BUY, OrderType::LIMIT, 10050, 100, "AAPL", 1});
  book.add({2, Side::BUY, OrderType::LIMIT, 10050, 200, "AAPL", 2});
  book.add({3, Side::BUY, OrderType::LIMIT, 10050, 300, "AAPL", 3});

  ASSERT(book.cancel(2));
  ASSERT(!book.getOrder(2).has_value());
  ASSERT(book.getOrder(1).has_value());
  ASSERT(book.getOrder(3).has_value());
  ASSERT(book.topBid()->id == 1);
}

void test_cancel_filled_order() {
  OrderBook book;

  book.add({1, Side::SELL, OrderType::LIMIT, 10000, 50, "AAPL", 1});
  book.reduceQuantity(1, 50);  // simulates full fill removal

  ASSERT(!book.cancel(1));
  ASSERT(book.getOrder(1) == std::nullopt);
}

void test_modify_quantity_down() {
  OrderBook book;

  book.add({1, Side::BUY, OrderType::LIMIT, 10000, 100, "AAPL", 1});

  ASSERT(book.modify(1, 40));
  ASSERT(book.getOrder(1)->quantity == 40);
  ASSERT(book.topBid()->quantity == 40);
}

void test_modify_to_zero_cancels() {
  OrderBook book;

  book.add({1, Side::SELL, OrderType::LIMIT, 10100, 75, "AAPL", 1});

  ASSERT(book.modify(1, 0));
  ASSERT(book.getOrder(1) == std::nullopt);
  ASSERT(!book.bestAsk().has_value());
}

void test_modify_price_change_requeues() {
  OrderBook book;

  book.add({1, Side::BUY, OrderType::LIMIT, 10000, 100, "AAPL", 1});
  book.add({2, Side::BUY, OrderType::LIMIT, 10025, 100, "AAPL", 2});

  ASSERT(book.modify(1, 100, 10050));
  ASSERT(book.getOrder(1)->price == 10050);
  ASSERT(book.bestBid() == 10050);
  ASSERT(book.topBid()->id == 1);
}

int main() {
  test_cancel_front_of_level();
  test_cancel_middle_of_level();
  test_cancel_filled_order();
  test_modify_quantity_down();
  test_modify_to_zero_cancels();
  test_modify_price_change_requeues();

  std::cout << tests_passed << "/" << tests_run << " tests passed\n";
  return tests_passed == tests_run ? 0 : 1;
}
