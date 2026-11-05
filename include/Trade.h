#pragma once
#include "Order.h"
#include <cstdint>
struct Trade{
    uint64_t tradeId;
    uint64_t timestamp;
    uint64_t price;
    uint64_t quantity;
    uint64_t buyOrderId;
    uint64_t sellOrderId;
    Side aggressorSide;
};