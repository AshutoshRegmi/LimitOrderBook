#pragma once

#include "Types.h"

struct Trade {
    OrderId buy_id;
    OrderId sell_id;
    Price price;
    Quantity quantity;
};
