#include "mpu.h"

void MPU::tick() {
    if (cycle == 0) {
        instr = mem->read(pc());
        pc_inc();
        cycle++;
    } else {
        // ADC, immediate
        if (instr == 0x69) {
            if (cycle == 1) {
                A += mem->read(pc()) + (P & Flag::C);
                pc_inc();
                cycle = 0;
            }
        }
    }
}
