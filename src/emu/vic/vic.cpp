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
        backgroundGraphics.cAccess();
    }
    backgroundGraphics.gAccess();

    VC = (VC + 1) & 0x3FF;
    VMLI = (VMLI + 1) & 0x3F;

    x += 8;
    if (x > lastX) {
        x = 0;
        y++;
    }
    // if (y > lastY) y = 0;
}

uint8_t VIC::read(uint16_t addr) {
    uint16_t effAddr = addr % 64;
    // Control register 1
    if (effAddr == 0x11) {
        return yScroll | (rSel << 3) | (displayEnable << 4) | (bitmapMode << 5) | (extendedColorMode << 6) | ((y >> 8) << 7);
    }
    // Raster
    else if (effAddr == 0x12) {
        return rasterCompareLine & 0x00FF;
    }
    // Control register 2
    else if (effAddr == 0x16) {
        return xScroll | (cSel << 3) | (multiColorMode << 4) | (1 << 5) | 0xC0;
    }
    // Memory pointers
    else if (effAddr == 0x18) {
        return 0x1 | ((charGenMemoryPosition & 0x07) << 1) | ((videoMatrixMemoryPosition & 0x0F) << 4);
    }
    // Interrupt register
    else if (effAddr == 0x19) {
        return rasterInterrupt | (spriteBitmapCollisionInterrupt << 1) | (spriteSpriteCollisionInterrupt << 2) | (lightpenInterrupt << 3) | 0x70 | (IRQ << 7);
    }
    // Interrupt enabled
    else if (effAddr == 0x1a) {
        return enableRasterInterrupt | (enableSpriteBitmapCollisionInterrupt << 1) | (enableSpriteSpriteCollisionInterrupt << 2) | (enableLightpenInterrupt << 3) | 0xF0;
    }
    // Border color
    else if (effAddr == 0x20) {
        return borderColor | 0xF0;
    }
    // Background color 0
    else if (effAddr == 0x20) {
        return backgroundColors[0] | 0xF0;
    }
    // Backgorund color 1
    else if (effAddr == 0x20) {
        return backgroundColors[1] | 0xF0;
    }
    // Backgorund color 2
    else if (effAddr == 0x20) {
        return backgroundColors[2] | 0xF0;
    }
    // Backgorund color 3
    else if (effAddr == 0x20) {
        return backgroundColors[3] | 0xF0;
    }
}

void VIC::write(uint16_t addr, uint8_t data) {

}
