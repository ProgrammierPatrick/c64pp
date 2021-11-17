#pragma once

#include "mpu.h"
#include "mem/ram_memory.h"
#include "mem/mpu_memory_view.h"

class C64 {
public:
    C64() : mpu(&mpuMemoryView), mainRAM(64 * 1024), mpuMemoryView(&mpu, &mainRAM) { }

private:
    MPU mpu;
    RAMMemory mainRAM;
    MPUMemoryView mpuMemoryView;
};

