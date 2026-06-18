#pragma once

#include "Order.h"
#include "Trade.h"

#include <vector>

class OrderBook;

class MatchingEngine {
public:
    explicit MatchingEngine(OrderBook& book);

    std::vector<Trade> process(const Order& order);

private:
    OrderBook& book_;
    void printTrade(const Trade& trade) const;
};
