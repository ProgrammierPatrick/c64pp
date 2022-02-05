#pragma once

#include <cstdint>

struct ColoredVal {
    uint8_t val;
    uint8_t col;
    ColoredVal(uint8_t val = 0, uint8_t col = 0) : val(val), col(col) { }
};
