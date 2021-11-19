#include "mpu_memory_view.h"

uint8_t MPUMemoryView::read(uint16_t addr) {
    // MCU IO Port
    if (addr == 0x0000) return 0x2F;
    if (addr == 0x0001) return bankSetting & 0x7;

    // ROMs
    if (addr >= 0xA000 && addr <= 0xBFFF && (bankSetting & 0x1))
        return basicROM->read(addr - 0xA000);
    if (addr >= 0xD000 && addr <= 0xDFFF && (bankSetting & 0x2))
        return kernalROM->read(addr - 0xD000);

    // Main RAM
    return mainRAM->read(addr);
}

void MPUMemoryView::write(uint16_t addr, uint8_t data) {
    // MCU IO Port
    if (addr == 0x0000) return;
    else if (addr == 0x0001) bankSetting = data & 0x7;

    // Main RAM
    else mainRAM->write(addr, data);
}
