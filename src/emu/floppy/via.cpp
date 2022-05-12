#include "via.h"

void VIA::tick() {

    // VIA U6: serial attention wired to CA1
    if (serialBus->getAttention())
        serialATNIRQ = true;
}

bool VIA::getIRQ() {
    return serialATNIRQEnable && serialATNIRQ;
}

uint8_t VIA::readVIA1(uint16_t addr, bool nonDestructive) {
    switch (addr) {
    case 0x0: // ORB
        return ((serialBus->getData()) ? 0x01 : 0x00) | ((serialBus->getClock()) ? 0x04 : 0x00) | ((serialBus->getAttention()) ? 0x80 : 0x00);
    case 0x1: // ORA
        if (!nonDestructive)
            serialATNIRQ = false; // ACK IRQ by reading from ORA
        return 0x00; // bit0: GND, others are not connected
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
        // TODO: PLA::ATNA = value & 0x10
        // TODO: PLA::ATN1 = value & 0x80
        return;
    case 0x1: // ORA
        serialATNIRQ = false; // ACK IRQ by writing to ORA
        return;
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
