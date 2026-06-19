#include "OrderBook.h"

#include <iostream>

int main() {
  OrderBook book;

  std::cout << "=== Day 3: Cancel + modify ===\n";

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

  return 0;
}
