#pragma once

#include "memory.h"
#include "../mpu.h"
#include "../io/cia.h"
#include "../vic/vic.h"

class MPUMemoryView : public Memory {
public:
    MPUMemoryView(MPU* mpu, Memory* mainRAM, Memory* colorRAM, Memory* basicROM, Memory* kernalROM, Memory* chargenROM, CIA* cia, VIC* vic)
        : mpu(mpu), mainRAM(mainRAM), colorRAM(colorRAM), basicROM(basicROM), kernalROM(kernalROM), chargenROM(chargenROM), cia(cia), vic(vic) { }

    uint8_t read(uint16_t addr) override;
    void write(uint16_t addr, uint8_t data) override;

    void reset() { bankSetting = 0x07; }

private:
    // 3 bits, connected to P0-P2 from MPU
    // bit 0: LORAM:  controls BASIC  rom (A000-BFFF)
    // bit 1: HIRAM:  controls KERNAL rom (E000-FFFF)
    // bit 2: CHAREN: controls IO/ROM (D000-DFFF)
    uint8_t bankSetting = 0x07;
    MPU* mpu;
    Memory* mainRAM;
    Memory* colorRAM;
    Memory* basicROM;
    Memory* kernalROM;
    Memory* chargenROM;
    CIA* cia;
    VIC* vic;
};
