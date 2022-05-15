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

    VIA(SerialBus *serialBus) : serialBus(serialBus) {
        serialBus->addDevice(this);
        serialDeviceName = "floppy";
    }

    void tick();

    uint8_t readVIA1(uint16_t addr, bool nonDestructive = false);
    uint8_t readVIA2(uint16_t addr, bool nonDestructive = false);
    void writeVIA1(uint16_t addr, uint8_t value);
    void writeVIA2(uint16_t addr, uint8_t value);

    bool getIRQ();

    // VIA 1 variables: serial interface
    bool serialATNLastTick = false;
    bool serialATNIRQ = false;
    bool serialATNIRQEnable = false;
    bool serialATNAutoACK = false; // called ATNA in schematic

    // VIA 2 variables: mechanical control
    uint8_t stepperMotorState = 0x00; // 2-bit stepper moter value. increasing: move head "upwards", decreasing: move head "downwards"
    bool spinMotorRunning = false;
    bool driveLED = false;
    bool driveSync = false; // 0 = Data bytes are being currently read from disk; 1 = SYNC marks are being read.
    uint8_t diskData = 0xAA; // TODO: repace with drive
    bool driveDDRAWriting = false; // 0: DDRA=00, 1: DDRA=FF
    bool driveHeadControlWrite = false; // from memorymap. 0:cause interrupts on negative CB2 transision 1:constant high output at CB2, no interrupts

    // VIA 2 timer
    bool timerRunning = false;

    // floppy disk stuff. TODO: split out into separate class
    bool diskWriteProtection = false;
    uint8_t diskDensity = 0x00; // 2-bit density. %00 = Lowest; %11 = Highest

private:
    SerialBus *serialBus = nullptr;


};
