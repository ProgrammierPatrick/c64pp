#include "vic.h"

void VIC::tick() {
    tickBackground();
    // tickBorder();

    x += 8;
    if (x > VIC::maxX) x = 0;
    cycleInLine++;
    if (cycleInLine == 63) {
        // TODO: interrupt here
    }

    if (cycleInLine == 64) {
        cycleInLine = 1;
        y++;
    }
    if (y > screenHeight) y = 0;
}

void VIC::tickBackground() {
    // isDisplayState: true when not in upper/lower border
    if (cycleInLine == 58 && RC == 7) {
        VCBASE = VC;
        inDisplayState = false;
        if (isBadLine()) RC++;
    }
    if (isBadLine()) inDisplayState = true;

    // new frame
    if (y == 0) {
        VCBASE = 0;
        displayEnableSetInThisFrame = false;
    }

    if (y == 0x30 && displayEnable)
        displayEnableSetInThisFrame = true;

    // start of border
    if (cycleInLine == 14) {
        VC = VCBASE;
        VMLI = 0;
        if (isBadLine()) RC = 0;
    }

    // MPU stunning
    BA = !(cycleInLine >= 12 && cycleInLine <= 54 && isBadLine());

    // to idle state
    if (cycleInLine == 58 && RC == 7) {
        VCBASE = VC;
        if (inDisplayState) RC = 0;
        else RC = (RC + 1) & 0x7;
        inDisplayState = false;
    }

    // clock low: VIC
    advanceGraphicsPipeline();
    if (cycleInLine >= 16 && cycleInLine <= 55){
        ColoredVal c = videoMatrixLine[VMLI];
        uint8_t g = 0;

        if (inDisplayState) {
            graphicsDataPipeline[0] = backgroundGraphics.gAccess();
        } else {
            //TODO: idle-mode g-access
        }

        VC = (VC + 1) & 0x3FF;
        VMLI = (VMLI + 1) & 0x3F;
    } else {
        // TODO: if no other access is done, idle-access is done
    }

    // high clock: MPU (when not stunned)
    if (!BA && cycleInLine >= 15 && cycleInLine <= 54) {
        // c-access: "to video matrix"
        backgroundGraphics.cAccess();
    }



}

void VIC::tickBorder() {
    if (x == (cSel ? 344 : 335))
        mainBorderFlipFlop = true;
    if (y == (rSel ? 51 : 55) && x == 8 * 63)
        verticalBorderFlipFlop = true;

    if (mainBorderFlipFlop)
        for(int i = 0; i < 8; i++)
            screen[y * screenWidth + cycleInLine * 8 + i] = borderColor;
}

void VIC::advanceGraphicsPipeline() {
    // graphics are drawn with two cycles delay. for xScroll, one further delayed value is needed
    if (inDisplayState && cycleInLine - 2 >= firstVisibleCycle && cycleInLine - 2 <= lastVisibleCycle) {
        for (int i = 0; i < 8; i++) {
            if (i < xScroll)
                screen[(y - firstVisibleY) * screenWidth + (cycleInLine - firstVisibleCycle - 2) * 8 + i] = graphicsDataPipeline[3][8 - xScroll + i];
            else
                screen[(y - firstVisibleY) * screenWidth + (cycleInLine - firstVisibleCycle - 2) * 8 + i] = graphicsDataPipeline[2][i];
        }
    }
    graphicsDataPipeline[3] = graphicsDataPipeline[2];
    graphicsDataPipeline[2] = graphicsDataPipeline[1];
    graphicsDataPipeline[1] = graphicsDataPipeline[0];
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
