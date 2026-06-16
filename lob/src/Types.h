#pragma once

#include <cstdint>

// Price in ticks: $100.50 with tick size $0.01 → 10050
using Price = int64_t;
using Quantity = uint32_t;
using OrderId = uint64_t;

enum class Side { BUY, SELL };
enum class OrderType { LIMIT, MARKET };
