#include "cia.h"

#include<iostream>

uint8_t CIA::read(uint16_t addr) {
    addr &= 0xFF0F;
    if (addr == 0x0000) return PRA;
    if (addr == 0x0001) return keyboard->query(PRA);
    if (addr == 0x0002) return DDRA;
    if (addr == 0x0003) return DDRB;
    if (addr == 0x0004) return static_cast<uint8_t>(timerCIA1.counterA & 0xFF);
    if (addr == 0x0005) return static_cast<uint8_t>(timerCIA1.counterA >> 8);
    if (addr == 0x0006) return static_cast<uint8_t>(timerCIA1.counterB & 0xFF);
    if (addr == 0x0007) return static_cast<uint8_t>(timerCIA1.counterB >> 8);
    if (addr == 0x000D) {
        auto tmp = ICRData1;
        ICRData1 = 0x00;
        return tmp;
    }
    if (addr == 0x000E) return timerCIA1.readCRA();
    if (addr == 0x000F) return timerCIA1.readCRB();

    if (addr == 0x0104) return static_cast<uint8_t>(timerCIA2.counterA & 0xFF);
    if (addr == 0x0105) return static_cast<uint8_t>(timerCIA2.counterA >> 8);
    if (addr == 0x0106) return static_cast<uint8_t>(timerCIA2.counterB & 0xFF);
    if (addr == 0x0107) return static_cast<uint8_t>(timerCIA2.counterB >> 8);
    if (addr == 0x010D) {
        auto tmp = ICRData2;
        ICRData2 = 0x00;
        return tmp;
    }
    if (addr == 0x010E) return timerCIA2.readCRA();
    if (addr == 0x010F) return timerCIA2.readCRB();
    return 0x00;
}

void CIA::write(uint16_t addr, uint8_t data) {
    addr &= 0xFF0F;
    if (addr == 0x0000) PRA = data;
    if (addr == 0x0002) std::cout << "Write on CIA1 DDRA ignored" << std::endl;
    if (addr == 0x0003) std::cout << "Write on CIA1 DDRB ignored" << std::endl;
    if (addr == 0x0004) timerCIA1.setTALO(data);
    if (addr == 0x0005) timerCIA1.setTAHI(data);
    if (addr == 0x0006) timerCIA1.setTBLO(data);
    if (addr == 0x0007) timerCIA1.setTBHI(data);
    if (addr == 0x000D) {
        if (data & 0x80) ICRData1 |= data & 0x7F;
        else             ICRData1 &= ~data;
    }
    if (addr == 0x000E) timerCIA1.writeCRA(data);
    if (addr == 0x000F) timerCIA1.writeCRB(data);

    if (addr == 0x0102) std::cout << "Write on CIA2 DDRA ignored" << std::endl;
    if (addr == 0x0103) std::cout << "Write on CIA2 DDRB ignored" << std::endl;
    if (addr == 0x0104) timerCIA2.setTALO(data);
    if (addr == 0x0105) timerCIA2.setTAHI(data);
    if (addr == 0x0106) timerCIA2.setTBLO(data);
    if (addr == 0x0107) timerCIA2.setTBHI(data);
    if (addr == 0x010D) {
        if (data & 0x80) ICRData2 |= data & 0x7F;
        else             ICRData2 &= ~data;
    }
    if (addr == 0x010E) timerCIA2.writeCRA(data);
    if (addr == 0x010F) timerCIA2.writeCRB(data);
}

void CIA::tick(bool& IRQ, bool& NMI) {
    IRQ = false;
    NMI = false;
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
