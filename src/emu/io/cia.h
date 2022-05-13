#pragma once

#include <vector>
#include <cstdint>

#include "keyboard.h"
#include "timer.h"
#include "serial.h"

#include "../mem/memory.h"

class CIA : public Memory, public SerialDevice {
public:
    CIA(Keyboard *keyboard, SerialBus *serialBus) : keyboard(keyboard), serialBus(serialBus) {
        serialBus->addDevice(this);
        serialDeviceName = "c64";
    }

    uint8_t read(uint16_t addr, bool nonDestructive = false) override;
    void write(uint16_t addr, uint8_t data) override;
    void tick();

    Timer timerCIA1;
    Timer timerCIA2;

    uint8_t vicBank = 0x0; // 2bit: upper 2 bits for VIC addresses

    //registers
    uint8_t PRA1 = 0xFF; // Peripheral Data Reg A (CIA 1)
    //uint8_t PRA2 = 0xFF; // Peripheral Data Reg A (CIA 2)
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
    SerialBus *serialBus;
};
