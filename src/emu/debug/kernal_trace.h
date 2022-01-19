#pragma once

#include "../mpu.h"

class KernalTrace {
public:
    KernalTrace(MPU* mpu) : mpu(mpu) { }

    void tick();

private:
    MPU* mpu;
};
