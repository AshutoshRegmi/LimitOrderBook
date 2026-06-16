#include "OrderBook.h"

#include <iostream>

int main() {
  OrderBook book;

  std::cout << "=== Day 1: Adding orders ===\n";

  // 5 buy orders (some at same price to show FIFO queue)
  book.add({1, Side::BUY, OrderType::LIMIT, 10050, 100, "AAPL", 1});  // $100.50
  book.add({2, Side::BUY, OrderType::LIMIT, 10050, 200, "AAPL", 2});  // $100.50
  book.add({3, Side::BUY, OrderType::LIMIT, 10025, 150, "AAPL", 3});  // $100.25
  book.add({4, Side::BUY, OrderType::LIMIT, 10000, 300, "AAPL", 4});  // $100.00
  book.add({5, Side::BUY, OrderType::LIMIT, 9950, 50, "AAPL", 5});    // $99.50

  // 5 sell orders
  book.add({6, Side::SELL, OrderType::LIMIT, 10100, 100, "AAPL", 6});  // $101.00
  book.add({7, Side::SELL, OrderType::LIMIT, 10125, 200, "AAPL", 7});  // $101.25
  book.add({8, Side::SELL, OrderType::LIMIT, 10125, 75, "AAPL", 8});   // $101.25
  book.add({9, Side::SELL, OrderType::LIMIT, 10150, 400, "AAPL", 9});  // $101.50
  book.add({10, Side::SELL, OrderType::LIMIT, 10200, 50, "AAPL", 10}); // $102.00

  book.print();

  std::cout << "=== Cancelling order 2 (middle of $100.50 bid level) ===\n";
  if (book.cancel(2)) {
    std::cout << "Order 2 cancelled.\n";
  } else {
    std::cout << "Failed to cancel order 2.\n";
  }

  book.print();

  return 0;
}
