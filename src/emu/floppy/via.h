#pragma once

#include "../io/serial.h"

#include <cstdint>

class VIA {
public:
    void tick();

    uint8_t read(uint8_t addr, bool nonDestructive = false);
    void write(uint16_t addr, uint8_t data);

    bool getIRQ() { return enableT1Interrupt && t1IRQ || enableCA1Interrupt && ca1IRQ; }

    uint8_t paIn = 0x00;
    uint8_t pbIn = 0x00;

    bool ca1In = false; // CA1 input. Will fire a CA1 interrupt on positive edge

    uint8_t paOut = 0x00;
    uint8_t pbOut = 0x00;

    bool lastCA1In = false;
    uint8_t paLatchValue = 0x00;
    uint16_t t1Counter = 0xFFFF;

    bool ca1IRQ = false;
    bool t1IRQ = false;

    // DDRB/DDRA 0:in, 1:out
    uint8_t ddrb = 0x00;
    uint8_t ddra = 0x00;

    // T1L-L/H
    uint16_t t1Latch = 0x0000;

    // ACR (auxiliary control register)
    bool paLatchingEnabled = false;
    bool t1FreeRunning = false; // 0:00 one-shot, 1:01 free running

    // PCR (peripheral control register): assume CA1 interrupt on positive edge (bit 0)
    bool ca2OutputHigh = false; // only support modes 0:000, 1:111. Since CA2 interrupts are not implemented, this is just a normal output pin
    bool cb2OutputHigh = false; // only support modes 0:000, 1:111. Since CB2 interrupts are not implemented, this is just a normal output pin

    // IRE (interrupt enable): only support CA1 and T1
    bool enableCA1Interrupt = false;
    bool enableT1Interrupt = false;
};
