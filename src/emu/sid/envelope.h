#pragma once

#include <cstdint>

class Envelope {
public:
    uint8_t tick();
    void setGate(bool gate);

    bool gate = false;

    uint8_t attack = 0;  // 4 bit value: Refer to table in data sheet for durations
    uint8_t decay = 0;   // 4 bit value: Refer to table in data sheet for durations
    uint8_t sustain = 0; // 4 bit value: 0h->0 Fh->1
    uint8_t release = 0; // 5 bit value: Refer to table in data sheet for durations

    uint16_t lfsr15 = 0x7FFF; // 15 bit LFSR for attack
    uint8_t  lfsr5  = 0x1F;   //  5 bit LFSR for decay / release

    uint8_t counter = 0x00;   // 8 bit "envelope DAC counter"
    bool countingUp = true;
    bool countingEnabled = false;
    uint8_t lfsr5Selection = 0x00; // 5 bit setting for lfsr5 exp decay approximation
};
