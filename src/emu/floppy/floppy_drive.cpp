#include "floppy_drive.h"

void FloppyDrive::tick() {

    // SO pin of 6502: set overflow (V) flag when CA1 of VIA U8 / BYTE RDY of 251828 goes low
    if (false) mpu.P |= MPU::Flag::V;

    mpu.tick(via.getIRQ());

    via.tick();
}
