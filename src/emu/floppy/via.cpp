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
    // 4-7
    case 0xB: // ACR (auxiliary control register)
        if (!nonDestructive) std::cout << "VIA1: read " << toHexStr(addr) << std::endl;
        return shiftRegMode1 << 2; // TODO: finish
    case 0xD: // IFR (interrupt flag register)
        return (serialATNIRQ && serialATNIRQEnable) ? 0x82 : 0x00;
    case 0xE: // IRE (interrupt enable)
        if (!nonDestructive) std::cout << "VIA1: read " << toHexStr(addr) << std::endl;
        return serialATNIRQEnable ? 0x02 : 0x00;
    }
    //if (!nonDestructive) std::cout << "VIA1: read " << toHexStr(addr) << std::endl;
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
    case 0xB: // ACR (auxiliary control register)
        std::cout << "VIA1 ACR = " << toHexStr(value) << std::endl;
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

    // std::cout << "VIA1: write [" << toHexStr(addr) << "] = " << toHexStr(value) << std::endl;
}

uint8_t VIA::readVIA2(uint16_t addr, bool nonDestructive) {
    if (!nonDestructive && addr != 0) std::cout << "VIA2: read " << toHexStr(addr) << std::endl;
    return 0x00;
}

void VIA::writeVIA2(uint16_t addr, uint8_t value) {
    if (addr != 0) std::cout << "VIA2: write [" << toHexStr(addr) << "] = " << toHexStr(value) << std::endl;
}
