#include "via.h"

#include "../text_utils.h"

#include <iostream>

void VIA::tick() {

    static bool lastATN = false, lastCLK = false, lastDATA = false;
    bool ATN = !serialBus->getAttention();
    bool CLK = !serialBus->getClock();
    bool DATA = !serialBus->getData();
    if (ATN != lastATN || CLK != lastCLK || DATA != lastDATA) {
        std::cout << "serial: ";
        if (ATN) {
            std::cout << "ATN[";
            for (auto d : serialBus->devices)
                if (d->pullAttention) std::cout << d->serialDeviceName;
            std::cout << "] ";
        }
        if (CLK) {
            std::cout << "CLK[";
            for (auto d : serialBus->devices)
                if (d->pullClock) std::cout << d->serialDeviceName;
            std::cout << "] ";
        }
        if (DATA) {
            std::cout << "DATA[";
            for (auto d : serialBus->devices)
                if (d->pullData) std::cout << d->serialDeviceName;
            std::cout << "] ";
        }
        std::cout << std::endl;
    }
    lastATN = ATN; lastCLK = CLK; lastDATA = DATA;

    if (!serialBus->getAttention() && serialATNLastTick) {
        serialATNIRQ = true;
        std::cout << "ATN received! enabled:" << (serialATNIRQEnable ? "true" : "false") << std::endl;
    }
    serialATNLastTick = serialBus->getAttention();

    // auto ATN acks
    pullData = !serialBus->getAttention() != serialATNAutoACK;
}

bool VIA::getIRQ() {
    return serialATNIRQEnable && serialATNIRQ;
}

uint8_t VIA::readVIA1(uint16_t addr, bool nonDestructive) {
    if (!nonDestructive) std::cout << "VIA1: read " << toHexStr(addr) << std::endl;
    switch (addr) {
    case 0x0: // ORB
        return ((serialBus->getData()) ? 0x03 : 0x00) | ((serialBus->getClock()) ? 0x0C : 0x00) | (serialATNAutoACK ? 0x10 : 0x00) | ((serialBus->getAttention()) ? 0x80 : 0x00);
    case 0x1: // ORA
        if (!nonDestructive)
            serialATNIRQ = false; // ACK IRQ by reading from ORA
        return 0x00; // bit0: GND, others are not connected
    case 0x2: // DDRB
        return 0x1A;
    case 0x3: // DDRA
        return 0xFF;
    case 0xC: // PCR (peripheral control register)
        return 0x01;
    case 0xD: // IFR (interrupt flag register)
        return (serialATNIRQ && serialATNIRQEnable) ? 0x82 : 0x00;
    case 0xE: // IRE (interrupt enable)
        if (!nonDestructive) std::cout << "VIA1: read " << toHexStr(addr) << std::endl;
        return serialATNIRQEnable ? 0x02 : 0x00;
    }
    if (!nonDestructive) std::cout << "VIA1: undefined read at " << toHexStr(addr) << std::endl;
    return 0x00;
}

void VIA::writeVIA1(uint16_t addr, uint8_t value) {
    std::cout << "VIA1: write [" << toHexStr(addr) << "] = " << toHexStr(value) << std::endl;
    switch (addr) {
    case 0x0: // ORB
        pullData = value & 0x02; // PB1
        pullClock = value & 0x08; // PB3
        serialATNAutoACK = value & 0x10; // PB4
        return;
    case 0x1: // ORA
        serialATNIRQ = false; // ACK IRQ by writing to ORA
        return;
    case 0x2: // DDRB
        if (value != 0x1A) std::cout << "VIA1: ignore setting DDRB to non-default value: "<< toHexStr(value) << std::endl;
        return;
    case 0x03: // DDRA
        if (value != 0xFF) std::cout << "VIA1: ignore setting DDRA to non-default value: "<< toHexStr(value) << std::endl;
        return;
    case 0xC: // PCR (peripheral control register)
        if (value != 0x01) std::cout << "VIA1: expected 0x01 for PCR write, but got " << toHexStr(value) << std::endl;
        // bit0: select rising or falling edge for CA1 interrupt. 1 = falling edge, hardcoded here
        return;
    case 0xD: // IFR (interrupt flag register)
        if (value & 0x02) serialATNIRQ = false;
        return;
    case 0xE: // IRE (interrupt enable)
        bool set = value & 0x80;
        if (set && (value & 0x02))
            serialATNIRQEnable = true;
        if (!set && (value & 0x02))
            serialATNIRQEnable = false;
        return;
    }
    std::cout << "VIA1: undefied write! [" << toHexStr(addr) << "] = " << toHexStr(value) << std::endl;
}

uint8_t VIA::readVIA2(uint16_t addr, bool nonDestructive) {
    if (!nonDestructive && addr != 0) std::cout << "VIA2: read " << toHexStr(addr) << std::endl;
    switch(addr) {
    case 0x0: // ORB
        return stepperMotorState | (spinMotorRunning ? 0x04 : 0) | (driveLED ? 0x08 : 0) | (!diskWriteProtection ? 0x10 : 0) | (diskDensity << 5) | (driveSync ? 0x80 : 0);
    case 0x1: // ORA
        // a new value is latched here whenever CA1 interrupt is fired.
        return diskData;
    case 0x2: // DDRB
        return 0x6F;
    case 0x3: // DDRA
        return driveDDRAWriting ? 0xFF : 0x00;
    case 0x0B: // ACR (auxiliary control register)
        return timerRunning ? 0x40 : 0x00;
    case 0x0C: // PCR (peripheral control register)
        return !driveHeadControlWrite ? 0x00 : 0xE0;
    }
    if (!nonDestructive) std::cout << "VIA2: undefined read at " << toHexStr(addr) << std::endl;
    return 0x00;
}

void VIA::writeVIA2(uint16_t addr, uint8_t value) {
    if (addr != 0) std::cout << "VIA2: write [" << toHexStr(addr) << "] = " << toHexStr(value) << std::endl;
    switch(addr) {
    case 0x0: // ORB
        stepperMotorState = value & 0x03;
        spinMotorRunning = value & 0x04;
        if (static_cast<bool>(value & 0x08) != driveLED)
            std::cout << "Drive LED: " << (value & 0x08 ? "ON" : "OFF") << std::endl;
        driveLED = value & 0x08;
        diskDensity = (value >> 5) & 0x03;
        return;
    case 0x1: // ORA
        if (driveDDRAWriting) diskData = value;
        return;
    case 0x2: // DDRB
        if (value != 0x6F) std::cout << "VIA2: ignore setting DDRB to non-default value: "<< toHexStr(value) << std::endl;
        return;
    case 0x3: // DDRA
        if (value != 0xFF && value != 0x00) std::cout << "VIA2: ignore setting DDRA to unknown value (known are: 00,FF): "<< toHexStr(value) << std::endl;
        driveDDRAWriting = value == 0xFF;
        return;
    case 0x0B: // ACR (auxiliary control register)
        if ((value & 0xFE) != 0x40) std::cout << "VIA2: expected 40,41 for ACR write, but got " << toHexStr(value) << std::endl;
        timerRunning = value & 0x40;
        return;
    case 0x0C: // PCR (peripheral control register)
        if (value >> 5 == 0) driveHeadControlWrite = false;
        else if (value >> 5 == 0x7) driveHeadControlWrite = true;
        else std::cout << "VIA2: write to VIA2 PCR sets unknown CB2 mode (write:" << toHexStr(value) << ", cb2 mode:" << toHexStr(static_cast<uint8_t>(value >> 5)) << ")" << std::endl;
        // driveCA2Enabled = false;
        std::cout << "write to VIA2 PCR: " << toHexStr(value) << std::endl;
        return;
    }
    std::cout << "VIA2: undefied write! [" << toHexStr(addr) << "] = " << toHexStr(value) << std::endl;
}
