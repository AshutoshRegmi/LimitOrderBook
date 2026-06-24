#include "Exchange.h"

#include "MatchingEngine.h"
#include "OrderBook.h"

std::vector<Trade> Exchange::process(const Order& order) {
    OrderBook& book = bookFor(order.symbol);
    MatchingEngine engine(book);
    engine.setVerbose(verbose_);
    return engine.process(order);
}

bool Exchange::cancel(const std::string& symbol, OrderId id) {
    auto it = books_.find(symbol);
    if (it == books_.end()) return false;
    return it->second.cancel(id);
}

bool Exchange::modify(const std::string& symbol, OrderId id, Quantity new_qty,
                      std::optional<Price> new_price) {
    auto it = books_.find(symbol);
    if (it == books_.end()) return false;
    return it->second.modify(id, new_qty, new_price);
}

std::optional<Order> Exchange::getOrder(const std::string& symbol, OrderId id) const {
    const OrderBook* book = findBook(symbol);
    if (!book) return std::nullopt;
    return book->getOrder(id);
}

const OrderBook* Exchange::book(const std::string& symbol) const {
    return findBook(symbol);
}

size_t Exchange::symbolCount() const {
    return books_.size();
}

OrderBook& Exchange::bookFor(const std::string& symbol) {
    return books_[symbol];
}

const OrderBook* Exchange::findBook(const std::string& symbol) const {
    const auto it = books_.find(symbol);
    if (it == books_.end()) return nullptr;
    return &it->second;
}
