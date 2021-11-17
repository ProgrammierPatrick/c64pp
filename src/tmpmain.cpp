#include "emu/mpu.h"
#include "emu/mem/ram_memory.h"

#include <iostream>

int main() {
    RAMMemory mem(16);
    mem.write(0, 0x69);
    mem.write(1, 3);
    mem.write(2, 0x69);
    mem.write(3, 5);
    MPU mpu(&mem);
    mpu.reset();
    mpu.PCL = 0;
    mpu.PCH = 0;
    mpu.A = 0;
    mpu.P = 1;
    mpu.tick();
    mpu.tick();
    mpu.P = 0;
    mpu.tick();
    mpu.tick();
    std::cout << "A: " << (int)mpu.A << std::endl;
}
