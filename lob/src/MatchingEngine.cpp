#include "MatchingEngine.h"

#include "OrderBook.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
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

}  // namespace

MatchingEngine::MatchingEngine(OrderBook& book) : book_(book) {}

std::vector<Trade> MatchingEngine::process(const Order& order) {
    std::vector<Trade> trades;
    Quantity remaining = order.quantity;

    if (order.side == Side::BUY) {
        while (remaining > 0) {
            auto ask = book_.topAsk();
            if (!ask) break;
            if (order.type == OrderType::LIMIT && order.price < ask->price) break;

            Quantity trade_qty = std::min(remaining, ask->quantity);
            Trade trade{order.id, ask->id, ask->price, trade_qty};
            trades.push_back(trade);
            printTrade(trade);

            book_.reduceQuantity(ask->id, trade_qty);
            remaining -= trade_qty;
        }
    } else {
        while (remaining > 0) {
            auto bid = book_.topBid();
            if (!bid) break;
            if (order.type == OrderType::LIMIT && order.price > bid->price) break;

            Quantity trade_qty = std::min(remaining, bid->quantity);
            Trade trade{bid->id, order.id, bid->price, trade_qty};
            trades.push_back(trade);
            printTrade(trade);

            book_.reduceQuantity(bid->id, trade_qty);
            remaining -= trade_qty;
        }
    }

    if (remaining > 0 && order.type == OrderType::LIMIT) {
        Order resting = order;
        resting.quantity = remaining;
        book_.add(resting);
    }

    return trades;
}

void MatchingEngine::printTrade(const Trade& trade) const {
    std::cout << "TRADE EXECUTED: " << trade.quantity << " shares @ $"
              << formatPrice(trade.price) << " | OrderID " << trade.buy_id
              << " x OrderID " << trade.sell_id << '\n';
}
