#pragma once

#include "memory.h"
#include "../mpu.h"

class MPUMemoryView : public Memory {
public:
    MPUMemoryView(MPU* mpu, Memory* mainRAM) : mpu(mpu), mainRAM(mainRAM) { }

    uint8_t read(uint16_t addr) override;
    void write(uint16_t addr, uint8_t data) override;

private:
    uint8_t bankSetting = 0x07; // 3 bits, connected to P0-P2 from MPU
    MPU* mpu;
    Memory* mainRAM;
    // TODO: add Kernal, charrom, etc
};
