#include "vic.h"

void VIC::tick() {

    // bool inDisplayState = !BA && x >= 8 * 15 && x <= 8 * 54;

    if (isBadLine()) inDisplayState = true;
    if (x == 8 * 58 && RC == 7 && !isBadLine())
        inDisplayState = false;

    // new frame
    if (y == 0) {
        VCBASE = 0;
    }

    // start of line
    if (x == 8 * 14) {
        VC = VCBASE;
        VMLI = 0;
        if (isBadLine()) RC = 0;
    }

    // MPU stunning
    BA = !(x >= 8 * 12 && x <= 8 * 54 && isBadLine());

    // to idle state
    if (x == 8 * 58 && RC == 7) {
        VCBASE = VC;
        if (inDisplayState) RC = 0;
        else RC = (RC + 1) & 0x7;
    }

    // c-access: "to video matrix"
    if (!BA && x >= 8 * 15 && x <= 8 * 54) {
        videoMatrixLine[VMLI] = readVM(VC);
    }
    uint8_t c = inDisplayState ? videoMatrixLine[VMLI] : 0;

    ColoredVal g = readCG(VC);

    VC = (VC + 1) & 0x3FF;
    VMLI = (VMLI + 1) & 0x3F;

    x += 8;
    if (x > lastX) {
        x = 0;
        y++;
    }
    // if (y > lastY) y = 0;
}
