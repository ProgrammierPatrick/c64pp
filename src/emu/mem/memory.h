#pragma once

#include <cstdint>

class Memory {
public:
    virtual ~Memory() { }
    virtual uint8_t read(uint16_t addr, bool nonDestructive = false) = 0;
    virtual void write(uint16_t addr, uint8_t data) = 0;
};
