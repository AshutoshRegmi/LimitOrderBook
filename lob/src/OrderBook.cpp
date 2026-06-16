#include "OrderBook.h"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace {

std::string formatPrice(Price ticks) {
    bool negative = ticks < 0;
    if (negative) ticks = -ticks;

    int64_t dollars = ticks / 100;
    int64_t cents = ticks % 100;

    std::ostringstream out;
    if (negative) out << '-';
    out << dollars << '.' << std::setw(2) << std::setfill('0') << cents;
    return out.str();
}

void printOrdersAtLevel(const std::list<Order>& orders) {
    for (const auto& order : orders) {
        std::cout << "    Order " << order.id << " | "
                  << order.quantity << " shares\n";
    }
}

}  // namespace

void OrderBook::add(const Order& order) {
    if (order.side == Side::BUY) {
        auto& queue = bids_[order.price];
        queue.push_back(order);
        order_index_[order.id] = {Side::BUY, order.price, std::prev(queue.end())};
    } else {
        auto& queue = asks_[order.price];
        queue.push_back(order);
        order_index_[order.id] = {Side::SELL, order.price, std::prev(queue.end())};
    }
}

bool OrderBook::cancel(OrderId id) {
    auto it = order_index_.find(id);
    if (it == order_index_.end()) {
        return false;
    }

    const OrderLocation loc = it->second;

    if (loc.side == Side::BUY) {
        auto level_it = bids_.find(loc.price);
        if (level_it == bids_.end()) return false;
        level_it->second.erase(loc.it);
        if (level_it->second.empty()) bids_.erase(level_it);
    } else {
        auto level_it = asks_.find(loc.price);
        if (level_it == asks_.end()) return false;
        level_it->second.erase(loc.it);
        if (level_it->second.empty()) asks_.erase(level_it);
    }

    order_index_.erase(it);
    return true;
}

std::optional<Price> OrderBook::bestBid() const {
    if (bids_.empty()) return std::nullopt;
    return bids_.begin()->first;
}

std::optional<Price> OrderBook::bestAsk() const {
    if (asks_.empty()) return std::nullopt;
    return asks_.begin()->first;
}

void OrderBook::print() const {
    std::cout << "\n========== ORDER BOOK ==========\n";

    std::cout << "\n--- ASKS (sells, low to high) ---\n";
    if (asks_.empty()) {
        std::cout << "  (empty)\n";
    } else {
        for (const auto& [price, orders] : asks_) {
            std::cout << "  $" << formatPrice(price) << "\n";
            printOrdersAtLevel(orders);
        }
    }

    std::cout << "\n--- BIDS (buys, high to low) ---\n";
    if (bids_.empty()) {
        std::cout << "  (empty)\n";
    } else {
        for (const auto& [price, orders] : bids_) {
            std::cout << "  $" << formatPrice(price) << "\n";
            printOrdersAtLevel(orders);
        }
    }

    if (bestBid() && bestAsk()) {
        std::cout << "\nBest bid: $" << formatPrice(*bestBid())
                  << " | Best ask: $" << formatPrice(*bestAsk()) << "\n";
    } else {
        std::cout << "\nBest bid: -- | Best ask: --\n";
    }

    std::cout << "================================\n\n";
}
