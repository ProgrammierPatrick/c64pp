#include "floppy_drive.h"

#include "../text_utils.h"
#include <iostream>

void FloppyDrive::tick() {

    // SO pin of 6502: set overflow (V) flag when CA1 of VIA U8 / BYTE RDY of 251828 goes low
    if (false) mpu.P |= MPU::Flag::V;

    static uint64_t tickCount = 0;
    if ((tickCount % 100000) == 0)
        std::cout << "Floppy Tick PC:" << toHexStr(mpu.PC) << std::endl;
    tickCount++;

    mpu.tick(via.getIRQ());
    via.tick();
}
