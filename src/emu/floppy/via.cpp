#include "via.h"

#include "../text_utils.h"

#include <iostream>

void VIA::tick() {

    // VIA U6: serial attention wired to CA1
    if (serialBus->getAttention())
        serialATNIRQ = true;

    static bool lastATN = false, lastCLK = false, lastDATA = false;
    bool ATN = serialBus->getAttention();
    bool CLK = serialBus->getClock();
    bool DATA = serialBus->getData();
    if ((ATN || CLK || DATA) && (ATN != lastATN || CLK != lastCLK || DATA != lastDATA))
        std::cout << "serial: " << (ATN ? "ATN " : "") << (CLK ? "CLK " : "") << (DATA ? "DATA " : "") << std::endl;
    lastATN = ATN; lastCLK = CLK; lastDATA = DATA;
}

bool VIA::getIRQ() {
    return serialATNIRQEnable && serialATNIRQ;
}

uint8_t VIA::readVIA1(uint16_t addr, bool nonDestructive) {
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
        return shiftRegMode1 << 2; // TODO: finish
    case 0xD: // IFR (interrupt flag register)
        return serialATNIRQ && serialATNIRQEnable ? 0x82 : 0x00;
    case 0xE: // IRE (interrupt enable)
        return serialATNIRQEnable ? 0x02 : 0x00;
    }
    return 0x00;
}

void VIA::writeVIA1(uint16_t addr, uint8_t value) {
    switch (addr) {
    case 0x0: // ORB
        pullData = !(value & 0x02);
        pullClock = !(value & 0x08);
        serialATNAutoACK = value & 0x10;
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
    case 0xE: // IRE (interrupt enable)
        if ((value & 0x82) == 0x82)
            serialATNIRQEnable = true;
        if ((value & 0x02) == 0x02)
            serialATNIRQEnable = false;
        return;
    }

}

uint8_t VIA::readVIA2(uint16_t addr, bool nonDestructive) {
    return 0x00;
}

void VIA::writeVIA2(uint16_t addr, uint8_t value) {

}
