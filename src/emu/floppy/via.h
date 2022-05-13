#pragma once

#include "../io/serial.h"

#include <cstdint>

/** emulates both VIA chips for the 1541-II Disk Drive
 *  Serial Data <- 6522U6 PB1, 251828 ATNE
 *  Serial Data -> 6522U6 PB0
 *  Serial Clock-> 6522U6 PB2
 *  Serial Clock<- 6522U6 PB3
 *  Serial ATN  -> 6522U6 PB4, CA1, 251828 ATN1
 *
 */
class VIA : public SerialDevice {
public:
    enum ShiftRegMode {
        Disabled,
        Timer2,
        SystemClock,
        External
    };

    VIA(SerialBus *serialBus) : serialBus(serialBus) {
        serialBus->addDevice(this);
    }

    void tick();

    uint8_t readVIA1(uint16_t addr, bool nonDestructive = false);
    uint8_t readVIA2(uint16_t addr, bool nonDestructive = false);
    void writeVIA1(uint16_t addr, uint8_t value);
    void writeVIA2(uint16_t addr, uint8_t value);

    bool getIRQ();

    bool serialATNIRQ = false;
    bool serialATNIRQEnable = false;
    bool serialATNAutoACK = false; // called ATNA in schematic

    ShiftRegMode shiftRegMode1;

private:
    SerialBus *serialBus = nullptr;


};
