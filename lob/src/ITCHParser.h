#pragma once

#include "Types.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

class Exchange;

struct ITCHParseStats {
    size_t messages = 0;
    size_t add_orders = 0;
    size_t deletes = 0;
    size_t replaces = 0;
    size_t executions = 0;
    size_t trade_messages = 0;
    size_t ignored = 0;
    size_t errors = 0;
    size_t trades_generated = 0;
};

class ITCHParser {
public:
    explicit ITCHParser(Exchange& exchange);

    ITCHParseStats replayFile(const std::string& path);
    bool processMessage(const uint8_t* data, size_t len);

private:
    struct OrderMeta {
        std::string symbol;
        Side side;
        uint64_t timestamp = 0;
    };

    bool processAdd(const uint8_t* data, size_t len, char type);
    bool processDelete(const uint8_t* data, size_t len);
    bool processReplace(const uint8_t* data, size_t len);
    bool processExecuted(const uint8_t* data, size_t len);
    bool processTrade(const uint8_t* data, size_t len);

    void trackOrder(uint64_t ref, const std::string& symbol, Side side, uint64_t timestamp);
    void untrackOrder(uint64_t ref);

    Exchange& exchange_;
    ITCHParseStats stats_;
    std::unordered_map<uint64_t, OrderMeta> order_meta_;
};
