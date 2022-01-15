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
        return yScroll | (rSel << 3) | (displayEnable << 4) | (bitmapMode << 5) | (extendedColorMode << 6) | ((rasterCompareLine >> 8) << 7);
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
    else if (effAddr == 0x1A) {
        return enableRasterInterrupt | (enableSpriteBitmapCollisionInterrupt << 1) | (enableSpriteSpriteCollisionInterrupt << 2) | (enableLightpenInterrupt << 3) | 0xF0;
    }
    // Border color
    else if (effAddr == 0x20) {
        return borderColor | 0xF0;
    }
    // Background color 0
    else if (effAddr == 0x21) {
        return backgroundColors[0] | 0xF0;
    }
    // Backgorund color 1
    else if (effAddr == 0x22) {
        return backgroundColors[1] | 0xF0;
    }
    // Backgorund color 2
    else if (effAddr == 0x23) {
        return backgroundColors[2] | 0xF0;
    }
    // Backgorund color 3
    else if (effAddr == 0x24) {
        return backgroundColors[3] | 0xF0;
    }
    else {
        return 0xFF;
    }
}

void VIC::write(uint16_t addr, uint8_t data) {
    uint16_t effAddr = addr % 64;
    // Control register 1
    if (effAddr == 0x11) {
        yScroll = data & 0x07;
        cSel = data & 0x08;
        displayEnable = data & 0x10;
        bitmapMode = data & 0x20;
        extendedColorMode = data & 0x40;
        rasterCompareLine |= (data >> 7) << 8;
    }
    // Raster counter
    else if (effAddr == 0x12) {
        rasterCompareLine = data;
    }
    // Control register 2
    else if (effAddr == 0x16) {
        xScroll = data & 0x07;
        cSel = data & 0x08;
        multiColorMode = data & 0x10;
    }
    // Memory pointers
    else if (effAddr == 0x18) {
        charGenMemoryPosition = data & 0x0E;
        videoMatrixMemoryPosition = data & 0xF0;
    }
    // Interrupt register
    else if (effAddr == 0x19) {
        rasterInterrupt = data & 0x01;
        spriteBitmapCollisionInterrupt = data & 0x02;
        spriteSpriteCollisionInterrupt = data & 0x04;
        lightpenInterrupt = data & 0x08;
        IRQ = data & 0x80;
    }
    // Interrupt enabled
    else if (effAddr == 0x1A) {
        enableRasterInterrupt = data & 0x01;
        enableSpriteBitmapCollisionInterrupt = data & 0x02;
        enableSpriteSpriteCollisionInterrupt = data & 0x04;
        enableLightpenInterrupt = data & 0x08;
    }
    // Border color
    else if (effAddr == 0x20) {
        borderColor = data & 0x0F;
    }
    // Background Color 0
    else if (effAddr == 0x21) {
        backgroundColors[0] = data & 0x0F;
    }
    // Background Color 1
    else if (effAddr == 0x22) {
        backgroundColors[1] = data & 0x0F;
    }
    // Background Color 2
    else if (effAddr == 0x23) {
        backgroundColors[2] = data & 0x0F;
    }
    // Background Color 3
    else if (effAddr == 0x24) {
        backgroundColors[3] = data & 0x0F;
    }
    else { }
}
