#pragma once

#include <cstdint>

class Keyboard {
public:
    uint8_t query(uint8_t mask) = 0;
};
