#pragma once

#include "Order.h"
#include "OrderBook.h"
#include "Trade.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Exchange {
public:
    std::vector<Trade> process(const Order& order);
    bool cancel(const std::string& symbol, OrderId id);
    bool modify(const std::string& symbol, OrderId id, Quantity new_qty,
                std::optional<Price> new_price = std::nullopt);
    void reduceQuantity(const std::string& symbol, OrderId id, Quantity amount);
    std::optional<Order> getOrder(const std::string& symbol, OrderId id) const;
    const OrderBook* book(const std::string& symbol) const;
    size_t symbolCount() const;
    void setVerbose(bool verbose) { verbose_ = verbose; }

private:
    OrderBook& bookFor(const std::string& symbol);
    const OrderBook* findBook(const std::string& symbol) const;

    std::unordered_map<std::string, OrderBook> books_;
    bool verbose_ = false;
};
