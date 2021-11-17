#include "mpu_memory_view.h"

uint8_t MPUMemoryView::read(uint16_t addr) {
    // MCU IO Port
    if (addr == 0x0000) return 0x2F;
    if (addr == 0x0001) return bankSetting & 0x7;

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
