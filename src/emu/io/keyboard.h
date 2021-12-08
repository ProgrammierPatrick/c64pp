#pragma once

#include <cstdint>

class Keyboard {
public:
    virtual ~Keyboard() { }
    virtual uint8_t query(uint8_t mask) = 0;
};
