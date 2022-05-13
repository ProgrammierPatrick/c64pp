#include "cia.h"

#include "../text_utils.h"

#include <iostream>

uint8_t CIA::read(uint16_t addr, bool nonDestructive) {
    addr &= 0xFF0F;
    switch (addr) {
        case 0x0000: return PRA1 & keyboard->queryJoystick2();
        case 0x0001: return keyboard->query(PRA1) & keyboard->queryJoystick1();
        case 0x0002: return DDRA;
        case 0x0003: return DDRB;
        case 0x0004: return static_cast<uint8_t>(timerCIA1.counterA & 0xFF);
        case 0x0005: return static_cast<uint8_t>(timerCIA1.counterA >> 8);
        case 0x0006: return static_cast<uint8_t>(timerCIA1.counterB & 0xFF);
        case 0x0007: return static_cast<uint8_t>(timerCIA1.counterB >> 8);
        case 0x000D: {
            auto tmp = ICRData1;
            if (nonDestructive == false) {
                ICRData1 = 0x00;
                IRQ = false;
            }
            return tmp;
        }
        case 0x000E: return timerCIA1.readCRA();
        case 0x000F: return timerCIA1.readCRB();

        case 0x0100: return ~vicBank & 0x03 | 0x04 | (pullAttention ? 0x08 : 0) | (pullClock ? 0x10 : 0) | (pullData ? 0x20 : 0) | (serialBus->getClock() ? 0x40 : 0) | (serialBus->getData() ? 0x80 : 0);
        case 0x0104: return static_cast<uint8_t>(timerCIA2.counterA & 0xFF);
        case 0x0105: return static_cast<uint8_t>(timerCIA2.counterA >> 8);
        case 0x0106: return static_cast<uint8_t>(timerCIA2.counterB & 0xFF);
        case 0x0107: return static_cast<uint8_t>(timerCIA2.counterB >> 8);
        case 0x010D: {
            auto tmp = ICRData2;
            if (nonDestructive == false) {
                ICRData2 = 0x00;
                NMI = false;
            }
            return tmp;
        }
        case 0x010E: return timerCIA2.readCRA();
        case 0x010F: return timerCIA2.readCRB();
        default: return 0x00;
    }
}

void CIA::write(uint16_t addr, uint8_t data) {
    addr &= 0xFF0F;
    switch (addr) {
        case 0x0000: PRA1 = data; break;
        // do not print these, can slow down execution due to console spam
        // case 0x0002: std::cout << "Write on CIA1 DDRA ignored" << std::endl; break;
        // case 0x0003: std::cout << "Write on CIA1 DDRB ignored" << std::endl; break;
        case 0x0004: timerCIA1.setTALO(data); break;
        case 0x0005: timerCIA1.setTAHI(data); break;
        case 0x0006: timerCIA1.setTBLO(data); break;
        case 0x0007: timerCIA1.setTBHI(data); break;
        case 0x000D: {
            if (data & 0x80) ICRMask1 |= data & 0x7F;
            else             ICRMask1 &= ~data;
            break;
        }
        case 0x000E: timerCIA1.writeCRA(data); break;
        case 0x000F: timerCIA1.writeCRB(data); break;

        case 0x0100: {
            vicBank = ~data & 0x03;
            pullAttention = data & 0x08;
            pullClock = data & 0x10;
            pullData = data & 0x20;
            break;
        }
        // do not print these, can slow down execution due to console spam
        // case 0x0102: std::cout << "Write on CIA2 DDRA ignored" << std::endl; break;
        // case 0x0103: std::cout << "Write on CIA2 DDRB ignored" << std::endl; break;
        case 0x0104: timerCIA2.setTALO(data); break;
        case 0x0105: timerCIA2.setTAHI(data); break;
        case 0x0106: timerCIA2.setTBLO(data); break;
        case 0x0107: timerCIA2.setTBHI(data); break;
        case 0x010D: {
            if (data & 0x80) ICRMask2 |= data & 0x7F;
            else             ICRMask2 &= ~data;
            break;
        }
        case 0x010E: timerCIA2.writeCRA(data); break;
        case 0x010F: timerCIA2.writeCRB(data); break;
    }
}

void CIA::tick() {
    bool underflowA1, underflowA2, underflowB1, underflowB2;
    timerCIA1.tick(underflowA1, underflowB1);
    timerCIA2.tick(underflowA2, underflowB2);
    if (underflowA1 && (ICRMask1 & 0x01)) {
        ICRData1 |= 0x81;
        IRQ = true;
    }
    if (underflowB1 && (ICRMask1 & 0x02)) {
        ICRData1 |= 0x82;
        IRQ = true;
    }
    if (underflowA2 && (ICRMask2 & 0x01)) {
        ICRData2 |= 0x81;
        NMI = true;
    }
    if (underflowB2 && (ICRMask2 & 0x02)) {
        ICRData2 |= 0x82;
        NMI = true;
    }

}
