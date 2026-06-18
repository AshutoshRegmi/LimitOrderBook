#include "MatchingEngine.h"
#include "OrderBook.h"

#include <iostream>

int main() {
  OrderBook book;
  MatchingEngine engine(book);

  // Scenario 1: simple cross — full fill
  std::cout << "=== Scenario 1: Simple cross ===\n";
  book.add({1, Side::SELL, OrderType::LIMIT, 10100, 100, "AAPL", 1});  // $101.00
  engine.process({2, Side::BUY, OrderType::LIMIT, 10100, 100, "AAPL", 2});
  book.print();

  // Scenario 2: partial fill — buy 100, only 60 available
  std::cout << "=== Scenario 2: Partial fill ===\n";
  book.add({3, Side::SELL, OrderType::LIMIT, 10100, 60, "AAPL", 3});
  engine.process({4, Side::BUY, OrderType::LIMIT, 10100, 100, "AAPL", 4});
  book.print();

  // Scenario 3: walk the book — buy crosses multiple ask levels
  std::cout << "=== Scenario 3: Walk the book ===\n";
  book.add({5, Side::SELL, OrderType::LIMIT, 10100, 50, "AAPL", 5});
  book.add({6, Side::SELL, OrderType::LIMIT, 10125, 50, "AAPL", 6});
  book.add({7, Side::SELL, OrderType::LIMIT, 10150, 50, "AAPL", 7});
  engine.process({8, Side::BUY, OrderType::LIMIT, 10200, 120, "AAPL", 8});
  book.print();

  // Scenario 4: no match — bid below ask, order rests
  std::cout << "=== Scenario 4: No match (order rests) ===\n";
  engine.process({9, Side::BUY, OrderType::LIMIT, 10000, 100, "AAPL", 9});  // $100.00
  book.print();

  // Scenario 5: market buy — takes best available ask
  std::cout << "=== Scenario 5: Market buy ===\n";
  engine.process({10, Side::BUY, OrderType::MARKET, 0, 30, "AAPL", 10});
  book.print();

  return 0;
}
