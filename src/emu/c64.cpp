#include "c64.h"

void C64::tick() {
    vic.tick();
    mpu.tick(cia.IRQ, cia.NMI);
    cia.tick();
}
