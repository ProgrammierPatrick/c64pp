#pragma once

#include <vector>
#include <cstdint>

#include "../mem/memory.h"
#include "keyboard.h"
#include "timer.h"

class CIA : public Memory {
public:
    CIA(Keyboard *keyboard) : keyboard(keyboard) { }

    uint8_t read(uint16_t addr) override;
    void write(uint16_t addr, uint8_t data) override;
    void tick();

    Timer timerCIA1;
    Timer timerCIA2;

    //registers
    uint8_t PRA = 0xFF; // Peripheral Data Reg A
    const uint8_t DDRA = 0xFF; // Data Direction Reg A
    const uint8_t DDRB = 0x00; // Data Direction Reg B
    //uint8_t TODTNTH; // 10ths Of Second Reg
    //uint8_t TODS; // Seconds Reg
    //uint8_t TODM; // Minutes Reg
    //uint8_t TODH; // Hours (AM/PM) Reg
    //uint8_t SDR; // Serial Data Reg
    // Interrupt Control Reg
    uint8_t ICRData1 = 0x00;
    uint8_t ICRData2 = 0x00;
    uint8_t ICRMask1 = 0x00;
    uint8_t ICRMask2 = 0x00;

    // interrupt outputs
    bool IRQ = false;
    bool NMI = false;

    Keyboard *keyboard;
};
