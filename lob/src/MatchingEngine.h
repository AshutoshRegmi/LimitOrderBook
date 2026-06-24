#pragma once

#include "Order.h"
#include "Trade.h"

#include <vector>

class OrderBook;

class MatchingEngine {
public:
    explicit MatchingEngine(OrderBook& book);

    std::vector<Trade> process(const Order& order);
    void setVerbose(bool verbose) { verbose_ = verbose; }

private:
    OrderBook& book_;
    bool verbose_ = false;
    void printTrade(const Trade& trade) const;
};
