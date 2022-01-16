#include "mpu_memory_view.h"

uint8_t MPUMemoryView::read(uint16_t addr) {
    // MCU IO Port
    if (addr == 0x0000) return 0x2F;
    if (addr == 0x0001) return bankSetting & 0x7;

    // ROMs
    if (addr >= 0xA000 && addr <= 0xBFFF && ((bankSetting & 0x3) == 0x3))
        return basicROM->read(addr - 0xA000);
    if (addr >= 0xD000 && addr <= 0xDFFF && (bankSetting & 0x3) != 0x0) {
        if (bankSetting & 0x4) {
            if (addr >= 0xD000 && addr <= 0xD3FF)
                return vic->read(addr - 0xD000);
            // missing: SID
            else if (addr >= 0xD800 && addr <= 0xDBE7)
                return colorRAM->read(addr - 0xD800) & 0x0F;
            else if (addr >= 0xDC00 && addr <= 0xDDFF)
                return cia->read(addr - 0xDC00);
            else return 0x00;
        }
        else {
            return chargenROM->read(addr - 0xD000);
        }
    }
    if (addr >= 0xE000 && addr <= 0xFFFF && (bankSetting & 0x2))
        return kernalROM->read(addr - 0xE000);

    // Main RAM
    return mainRAM->read(addr);
}

void MPUMemoryView::write(uint16_t addr, uint8_t data) {
    // MCU IO Port
    if (addr == 0x0000) return;
    else if (addr == 0x0001) bankSetting = data & 0x7;

    // IO region
    else if (addr >= 0xD000 && addr <= 0xDFFF && (bankSetting & 0x3) != 0x0 && (bankSetting & 0x4)) {
        if (addr >= 0xD000 && addr <= 0xD3FF)
            vic->write(addr - 0xD000, data);
        // missing: SID
        if (addr >= 0xD800 && addr<= 0xDBE7)
            colorRAM->write(addr - 0xD800, data);
        if (addr >= 0xDC00 && addr <= 0xDDFF)
            cia->write(addr - 0xDC00, data);
    }

    // Main RAM
    else mainRAM->write(addr, data);
}
