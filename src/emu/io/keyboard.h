#pragma once

#include <cstdint>

class Keyboard {
public:
    virtual ~Keyboard() { }
    virtual uint8_t query(uint8_t mask) = 0;
    virtual bool queryRestore() = 0;
    virtual uint8_t queryJoystick1() = 0;
    virtual uint8_t queryJoystick2() = 0;
};
