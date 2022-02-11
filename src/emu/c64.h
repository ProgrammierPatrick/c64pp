#pragma once

#include "mpu.h"
#include "mem/ram_memory.h"
#include "mem/rom_memory.h"
#include "mem/mpu_memory_view.h"
#include "io/cia.h"
#include "vic/vic.h"
#include "debug/mpu_trace.h"
#include "debug/kernal_trace.h"
#include "debug/break_points.h"

class C64 {
public:
    C64(const std::vector<uint8_t>& basicROM, const std::vector<uint8_t>& kernalROM, const std::vector<uint8_t>& chargenROM, Keyboard* keyboard)
        : mpu(&mpuMemoryView),
          mainRAM(64 * 1024), colorRAM(1024),
          mpuMemoryView(&mpu, &mainRAM, &colorRAM, &this->basicROM, &this->kernalROM, &this->chargenROM, &cia, &vic),
          basicROM(basicROM), kernalROM(kernalROM), chargenROM(chargenROM),
          cia(keyboard),
          vic(&mainRAM, &this->chargenROM, &colorRAM, &cia),
          mpuTrace(&mpu), kernalTrace(&mpu) {
        reset();
    }

    void tick();

    void reset() {
        mpu.reset();
    }

public:
    MPU mpu;
    RAMMemory mainRAM;
    RAMMemory colorRAM;
    ROMMemory basicROM;
    ROMMemory kernalROM;
    ROMMemory chargenROM;
    MPUMemoryView mpuMemoryView;
    CIA cia;
    VIC vic;
    MPUTrace mpuTrace;
    KernalTrace kernalTrace;
    BreakPoints breakPoints;

    bool mpuStunned = false;
};

