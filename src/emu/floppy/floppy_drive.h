#pragma once

#include "via.h"

#include "../io/serial.h"
#include "../mem/memory.h"
#include "../mem/ram_memory.h"
#include "../mem/rom_memory.h"
#include "../mpu.h"

class FloppyDriveMemoryView : public Memory {
public:
    FloppyDriveMemoryView(RAMMemory *ram, ROMMemory *dos, VIA *via) : ram(ram), dos(dos), via(via) {}
    uint8_t read(uint16_t addr, bool nonDestructive = false) {
        if (addr < 0x0800) return ram->read(addr, nonDestructive);
        else if (addr >= 0x1800 && addr < 0x1810) return via->readVIA1(addr - 0x1800, nonDestructive);
        else if (addr >= 0x1C00 && addr < 0x1C10) return via->readVIA2(addr - 0x1C00, nonDestructive);
        else if (addr >= 0xC000) return dos->read(addr - 0xC000, nonDestructive);
        return 0x00;
    }
    void write(uint16_t addr, uint8_t data) {
        if (addr < 0x800) ram->write(addr, data);
        else if (addr >= 0x1800 && addr < 0x1810) via->writeVIA1(addr - 0x1800, data);
        else if (addr >= 0x1C00 && addr < 0x1C10) via->writeVIA2(addr - 0x1C00, data);
        else if (addr >= 0xC000) dos->write(addr - 0xC000, data);
    }
    RAMMemory *ram;
    ROMMemory *dos;
    VIA *via;
};

class FloppyDrive {
public:
    FloppyDrive(SerialBus *serialBus, const std::vector<uint8_t>& dosROM)
        : via(serialBus), mpu(&memoryView), ram(0x800), dos(dosROM),
          memoryView(&ram, &dos, &via) {
        mpu.reset();
    }

    void tick();

    MPU mpu;
    RAMMemory ram;
    ROMMemory dos;
    VIA via;
    FloppyDriveMemoryView memoryView;
};
