#pragma once

#include "Types.h"

#include <string>

struct Order {
    OrderId id;
    Side side;
    OrderType type;
    Price price;
    Quantity quantity;
    std::string symbol;
    uint64_t timestamp;
};
