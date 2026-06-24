#include "ITCHParser.h"

#include "Exchange.h"
#include "Order.h"

#include <fstream>
#include <iostream>
#include <vector>

namespace {

constexpr size_t kAddOrderNoMpidSize = 36;
constexpr size_t kAddOrderMpidSize = 40;
constexpr size_t kOrderDeleteSize = 19;
constexpr size_t kOrderReplaceSize = 35;
constexpr size_t kOrderExecutedSize = 31;
constexpr size_t kTradeMessageSize = 44;

uint16_t readU16BE(const uint8_t* data) {
    return static_cast<uint16_t>((data[0] << 8) | data[1]);
}

uint32_t readU32BE(const uint8_t* data) {
    return (static_cast<uint32_t>(data[0]) << 24) |
           (static_cast<uint32_t>(data[1]) << 16) |
           (static_cast<uint32_t>(data[2]) << 8) | data[3];
}

uint64_t readU64BE(const uint8_t* data) {
    return (static_cast<uint64_t>(data[0]) << 56) |
           (static_cast<uint64_t>(data[1]) << 48) |
           (static_cast<uint64_t>(data[2]) << 40) |
           (static_cast<uint64_t>(data[3]) << 32) |
           (static_cast<uint64_t>(data[4]) << 24) |
           (static_cast<uint64_t>(data[5]) << 16) |
           (static_cast<uint64_t>(data[6]) << 8) | data[7];
}

uint64_t readU48BE(const uint8_t* data) {
    return (static_cast<uint64_t>(data[0]) << 40) |
           (static_cast<uint64_t>(data[1]) << 32) |
           (static_cast<uint64_t>(data[2]) << 24) |
           (static_cast<uint64_t>(data[3]) << 16) |
           (static_cast<uint64_t>(data[4]) << 8) | data[5];
}

Price itchPriceToTicks(uint32_t itch_price) {
    return static_cast<Price>(itch_price / 100);
}

std::string readAlpha(const uint8_t* data, size_t len) {
    std::string value(reinterpret_cast<const char*>(data), len);
    while (!value.empty() && value.back() == ' ') {
        value.pop_back();
    }
    return value;
}

bool isMessageType(char c) {
    switch (c) {
        case 'A':
        case 'F':
        case 'D':
        case 'U':
        case 'E':
        case 'P':
        case 'S':
        case 'R':
        case 'X':
        case 'C':
        case 'Q':
        case 'B':
            return true;
        default:
            return false;
    }
}

}  // namespace

ITCHParser::ITCHParser(Exchange& exchange) : exchange_(exchange) {}

void ITCHParser::trackOrder(uint64_t ref, const std::string& symbol, Side side,
                            uint64_t timestamp) {
    order_meta_[ref] = {symbol, side, timestamp};
}

void ITCHParser::untrackOrder(uint64_t ref) {
    order_meta_.erase(ref);
}

bool ITCHParser::processAdd(const uint8_t* data, size_t len, char type) {
    const size_t expected = type == 'F' ? kAddOrderMpidSize : kAddOrderNoMpidSize;
    if (len < expected) {
        ++stats_.errors;
        return false;
    }

    const uint64_t timestamp = readU48BE(data + 5);
    const uint64_t order_ref = readU64BE(data + 11);
    const char side_char = static_cast<char>(data[19]);
    const uint32_t shares = readU32BE(data + 20);
    const std::string symbol = readAlpha(data + 24, 8);
    const Price price = itchPriceToTicks(readU32BE(data + 32));

    const Side side = side_char == 'B' ? Side::BUY : Side::SELL;
    const Order order{order_ref,
                      side,
                      OrderType::LIMIT,
                      price,
                      shares,
                      symbol,
                      timestamp};

    const auto trades = exchange_.process(order);
    stats_.trades_generated += trades.size();
    trackOrder(order_ref, symbol, side, timestamp);
    ++stats_.add_orders;
    return true;
}

bool ITCHParser::processDelete(const uint8_t* data, size_t len) {
    if (len < kOrderDeleteSize) {
        ++stats_.errors;
        return false;
    }

    const uint64_t order_ref = readU64BE(data + 11);
    const auto it = order_meta_.find(order_ref);
    if (it == order_meta_.end()) {
        ++stats_.errors;
        return false;
    }

    const bool ok = exchange_.cancel(it->second.symbol, order_ref);
    if (ok) {
        untrackOrder(order_ref);
        ++stats_.deletes;
    } else {
        ++stats_.errors;
    }
    return ok;
}

bool ITCHParser::processReplace(const uint8_t* data, size_t len) {
    if (len < kOrderReplaceSize) {
        ++stats_.errors;
        return false;
    }

    const uint64_t original_ref = readU64BE(data + 11);
    const uint64_t new_ref = readU64BE(data + 19);
    const uint32_t shares = readU32BE(data + 27);
    const Price price = itchPriceToTicks(readU32BE(data + 31));

    const auto it = order_meta_.find(original_ref);
    if (it == order_meta_.end()) {
        ++stats_.errors;
        return false;
    }

    const OrderMeta meta = it->second;
    if (!exchange_.cancel(meta.symbol, original_ref)) {
        ++stats_.errors;
        return false;
    }
    untrackOrder(original_ref);

    const Order replacement{new_ref,
                            meta.side,
                            OrderType::LIMIT,
                            price,
                            shares,
                            meta.symbol,
                            readU48BE(data + 5)};

    const auto trades = exchange_.process(replacement);
    stats_.trades_generated += trades.size();
    trackOrder(new_ref, meta.symbol, meta.side, replacement.timestamp);
    ++stats_.replaces;
    return true;
}

bool ITCHParser::processExecuted(const uint8_t* data, size_t len) {
    if (len < kOrderExecutedSize) {
        ++stats_.errors;
        return false;
    }

    const uint64_t order_ref = readU64BE(data + 11);
    const uint32_t executed_shares = readU32BE(data + 19);

    const auto it = order_meta_.find(order_ref);
    if (it == order_meta_.end()) {
        ++stats_.ignored;
        return true;
    }

    exchange_.reduceQuantity(it->second.symbol, order_ref, executed_shares);
    if (!exchange_.getOrder(it->second.symbol, order_ref).has_value()) {
        untrackOrder(order_ref);
    }
    ++stats_.executions;
    return true;
}

bool ITCHParser::processTrade(const uint8_t* /*data*/, size_t len) {
    if (len < kTradeMessageSize) {
        ++stats_.errors;
        return false;
    }
    ++stats_.trade_messages;
    return true;
}

bool ITCHParser::processMessage(const uint8_t* data, size_t len) {
    if (len == 0) {
        ++stats_.errors;
        return false;
    }

    ++stats_.messages;
    const char type = static_cast<char>(data[0]);

    switch (type) {
        case 'A':
            return processAdd(data, len, 'A');
        case 'F':
            return processAdd(data, len, 'F');
        case 'D':
            return processDelete(data, len);
        case 'U':
            return processReplace(data, len);
        case 'E':
            return processExecuted(data, len);
        case 'P':
            return processTrade(data, len);
        default:
            ++stats_.ignored;
            return true;
    }
}

ITCHParseStats ITCHParser::replayFile(const std::string& path) {
    stats_ = {};
    order_meta_.clear();

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::cerr << "Failed to open ITCH file: " << path << '\n';
        ++stats_.errors;
        return stats_;
    }

    std::vector<uint8_t> file_data((std::istreambuf_iterator<char>(in)),
                                   std::istreambuf_iterator<char>());
    if (file_data.empty()) {
        ++stats_.errors;
        return stats_;
    }

    size_t offset = 0;
    const bool length_prefixed =
        file_data.size() >= 3 &&
        readU16BE(file_data.data()) + 2 <= file_data.size() &&
        isMessageType(static_cast<char>(file_data[2]));

    if (length_prefixed) {
        while (offset + 2 <= file_data.size()) {
            const uint16_t msg_len = readU16BE(file_data.data() + offset);
            offset += 2;
            if (msg_len == 0 || offset + msg_len > file_data.size()) {
                ++stats_.errors;
                break;
            }
            processMessage(file_data.data() + offset, msg_len);
            offset += msg_len;
        }
        return stats_;
    }

    while (offset + 20 <= file_data.size()) {
        const uint16_t count = readU16BE(file_data.data() + offset + 18);
        offset += 20;
        for (uint16_t i = 0; i < count; ++i) {
            if (offset + 2 > file_data.size()) {
                ++stats_.errors;
                return stats_;
            }
            const uint16_t msg_len = readU16BE(file_data.data() + offset);
            offset += 2;
            if (msg_len == 0 || offset + msg_len > file_data.size()) {
                ++stats_.errors;
                return stats_;
            }
            processMessage(file_data.data() + offset, msg_len);
            offset += msg_len;
        }
    }

    return stats_;
}
