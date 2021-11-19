#pragma once

#include "mpu.h"
#include "mem/ram_memory.h"
#include "mem/rom_memory.h"
#include "mem/mpu_memory_view.h"

class C64 {
public:
    C64(const std::vector<uint8_t>& basicROM, const std::vector<uint8_t>& kernalROM, const std::vector<uint8_t>& chargenROM)
        : mpu(&mpuMemoryView), mainRAM(64 * 1024),
          mpuMemoryView(&mpu, &this->mainRAM, &this->basicROM, &this->kernalROM),
          basicROM(basicROM), kernalROM(kernalROM) {
        mpu.reset();
    }

public:
    MPU mpu;
    RAMMemory mainRAM;
    ROMMemory basicROM;
    ROMMemory kernalROM;
    MPUMemoryView mpuMemoryView;
};

