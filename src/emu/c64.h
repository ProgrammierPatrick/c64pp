#pragma once

#include "mpu.h"
#include "mem/ram_memory.h"
#include "mem/rom_memory.h"
#include "mem/mpu_memory_view.h"
#include "io/cia.h"

class C64 {
public:
    C64(const std::vector<uint8_t>& basicROM, const std::vector<uint8_t>& kernalROM, const std::vector<uint8_t>& chargenROM, Keyboard* keyboard)
        : mpu(&mpuMemoryView), mainRAM(64 * 1024),
          mpuMemoryView(&mpu, &this->mainRAM, &this->basicROM, &this->kernalROM, &this->chargenROM, &this->cia),
          basicROM(basicROM), kernalROM(kernalROM), chargenROM(chargenROM),
          cia(keyboard) {
        reset();
    }

    void tick();

    void reset() {
        mpu.reset();
    }

public:
    MPU mpu;
    RAMMemory mainRAM;
    ROMMemory basicROM;
    ROMMemory kernalROM;
    ROMMemory chargenROM;
    MPUMemoryView mpuMemoryView;
    CIA cia;
};

