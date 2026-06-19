#pragma once

#include "Order.h"

#include <list>
#include <map>
#include <optional>
#include <unordered_map>

class OrderBook {
public:
    void add(const Order& order);
    bool cancel(OrderId id);
    bool modify(OrderId id, Quantity new_qty, std::optional<Price> new_price = std::nullopt);
    std::optional<Order> getOrder(OrderId id) const;
    void print() const;

    std::optional<Price> bestBid() const;
    std::optional<Price> bestAsk() const;

    std::optional<Order> topBid() const;
    std::optional<Order> topAsk() const;
    void reduceQuantity(OrderId id, Quantity amount);

private:
    using OrderList = std::list<Order>;

    struct OrderLocation {
        Side side;
        Price price;
        OrderList::iterator it;
    };

    // Bids: highest price first
    std::map<Price, OrderList, std::greater<Price>> bids_;
    // Asks: lowest price first
    std::map<Price, OrderList> asks_;
    // Fast lookup for cancellation
    std::unordered_map<OrderId, OrderLocation> order_index_;
};
