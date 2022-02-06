#include "vic.h"

#include "../io/cia.h"

void VIC::tick() {
    tickBackground();
    tickBorder();

    // compare y with rasterCompareLine
    if (y == 0 && cycleInLine == 2 || y != 0 && cycleInLine == 1) {
        if (y == rasterCompareLine) rasterInterrupt = true; // raster interrupt (RST)
    }

    x += 8;
    if (x > VIC::maxX) x = 0;
    cycleInLine++;
    if (cycleInLine == 63) {
        // TODO: interrupt here
    }

    if (cycleInLine == 64) {
        cycleInLine = 1;
        y++;
        if (y >= lastY) {
            y = 0;
            // for(int i = 0; i < screenWidth * screenHeight; i++) screen[i] = 0;
        }
    }

    checkIRQ();
}

void VIC::tickBackground() {
    // std::cout << "y:" << (int)y << " x:" << (int)cycleInLine << " state:" << (inDisplayState ? "display" : "idle") << " RC:" << (int)RC
    //     << " VMLI:" << (int)VMLI << " VCBASE:" << (int)VCBASE << " VC:" << (int)VC << std::endl;


    if (cycleInLine == 9) {
        VMLI = 0;
    }
    if (cycleInLine == 11) {
        VC = VCBASE;
    }
    if (cycleInLine == 14 && inDisplayState && isBadLine()) {
        RC = 0;
    }
    if (cycleInLine >= 16 && cycleInLine <= 55 && inDisplayState) {
        auto gPixels = backgroundGraphics.gAccess(videoMatrixLine[VMLI], VC, RC);
        graphicsDataPipeline[0] = gPixels;
        advanceGraphicsPipeline();

        // draw character bad line in alternating colors
        // if (RC == 0) for (int i = 0; i < 8; i++)
        //     screen[(y - firstVisibleY) * screenWidth + (cycleInLine - firstVisibleCycle) * 8 + i] = (VMLI % 2) ? 1 : 2;

        VC++;
        VMLI++;
    }
    if (cycleInLine >= 15 && cycleInLine <= 54 && inDisplayState && isBadLine()) {
        backgroundGraphics.cAccess();
    }
    if (cycleInLine == 58 && (!inDisplayState || RC == 7)) {
        VCBASE = VC;
    }
    if (cycleInLine == 58 && RC == 7) {
        inDisplayState = false;
    }
    if (cycleInLine == 58 && (RC < 7 || isBadLine())) {
        RC++;
    }

    if (y >= 0 && y <= 0x2F) {
        VCBASE = 0;
    }
    if (y == 0x30 && !displayEnable && cycleInLine == 1)
        displayEnableSetInThisFrame = false;
    if (y == 0x30 && displayEnable)
        displayEnableSetInThisFrame = true;

    if (isBadLine()) inDisplayState = true;
}

void VIC::tickBorder() {
    // if mainFF set, output border color
    // if verticalFF set, bg-sequencer outputs background color (related to sprite collisions)

    int top = rSel ? 51 : 55;
    int bottom = rSel ? 251 : 247;
    int left = cSel ? 24 : 31;
    int right = cSel ? 344 : 335;

    for(int i = 0; i < 8; i++) {
        int xx = firstCycleX + (cycleInLine - 1) * 8 + i;
        if (xx > maxX) xx -= maxX + 1;

        if (xx == right)
            mainBorderFlipFlop = true;
        if (y == bottom && cycleInLine == 63)
            verticalBorderFlipFlop = true;
        if (y == top && cycleInLine == 63 && displayEnable)
            verticalBorderFlipFlop = false;
        if (xx == left && y == bottom)
            verticalBorderFlipFlop = true;
        if (xx == left && y == top && displayEnable)
            verticalBorderFlipFlop = false;
        if (xx == left && !verticalBorderFlipFlop)
            mainBorderFlipFlop = false;

        int sy = y - firstVisibleY;
        int sx = (cycleInLine - firstVisibleCycle) * 8 + i;
        if (mainBorderFlipFlop && sy >= 0 && sy < screenHeight && sx >= 0  && sx < screenWidth)
            screen[sy * screenWidth + sx] = borderColor;
    }
}

void VIC::advanceGraphicsPipeline() {
    int delay = 0; // 2
    // graphics are drawn with two cycles delay. for xScroll, one further delayed value is needed
    if (inDisplayState) {
        for (int i = 0; i < 8; i++) {
            int sy = y - firstVisibleY;
            int sx = (cycleInLine - firstVisibleCycle - delay) * 8 + i + 4;
            if (sy >= 0 && sy < screenHeight && sx >= 0  && sx < screenWidth) {
                if (i < xScroll)
                    screen[sy * screenWidth + sx] = graphicsDataPipeline[1][i - xScroll + 8];
                else
                    screen[sy * screenWidth + sx] = graphicsDataPipeline[0][i - xScroll];
            }
        }
    }
    graphicsDataPipeline[3] = graphicsDataPipeline[2];
    graphicsDataPipeline[2] = graphicsDataPipeline[1];
    graphicsDataPipeline[1] = graphicsDataPipeline[0];
}


ColoredVal VIC::accessMem(uint16_t addr) {
    uint8_t bankSetting = ~cia->PRA2 & 0x03;

    uint8_t val;
    if (((bankSetting % 2) == 0) && addr >= 0x1000 && addr <= 0x1FFF)
        val = charROM->read(addr & 0x0FFF);
    else {
        uint16_t absAddr = (bankSetting << 14) | addr;
        val = mainRAM->read(absAddr);
    }
    return ColoredVal(val, colorRAM->read(addr & 0x03FF));
}

uint8_t VIC::read(uint16_t addr, bool nonDestructive) {
    uint16_t effAddr = addr % 64;
    switch (effAddr) {
        // X coordinate sprite 0
        case 0x00:
            return sprites[0].xCoord & 0x00FF;
        // Y coordinate sprite 0
        case 0x01:
            return sprites[0].yCoord & 0x00FF;
        // X coordinate sprite 1
        case 0x02:
            return sprites[1].xCoord & 0x00FF;
        // Y coordinate sprite 1
        case 0x03:
            return sprites[1].yCoord & 0x00FF;
        // X coordinate sprite 2
        case 0x04:
            return sprites[2].xCoord & 0x00FF;
        // Y coordinate sprite 2
        case 0x05:
            return sprites[2].yCoord & 0x00FF;
        // X coordinate sprite 3
        case 0x06:
            return sprites[3].xCoord & 0x00FF;
        // Y coordinate sprite 3
        case 0x07:
            return sprites[3].yCoord & 0x00FF;
        // X coordinate sprite 4
        case 0x08:
            return sprites[4].xCoord & 0x00FF;
        // Y coordinate sprite 4
        case 0x09:
            return sprites[4].yCoord & 0x00FF;
        // X coordinate sprite 5
        case 0x0A:
            return sprites[5].xCoord & 0x00FF;
        // Y coordinate sprite 5
        case 0x0B:
            return sprites[5].yCoord & 0x00FF;
        // X coordinate sprite 6
        case 0x0C:
            return sprites[6].xCoord & 0x00FF;
        // Y coordinate sprite 6
        case 0x0D:
            return sprites[6].yCoord & 0x00FF;
        // X coordinate sprite 7
        case 0x0E:
            return sprites[7].xCoord & 0x00FF;
        // Y coordinate sprite 7
        case 0x0F:
            return sprites[7].yCoord & 0x00FF;
        // MSBs of X coordinates
        case 0x10: {
            uint8_t msbs = 0;
            for (int i = 0; i < 8; i++) {
                msbs |= ((sprites[i].xCoord >> 8) & 0x01) << i;
            }
            return msbs;
        }
        // Control register 1
        case 0x11:
            return yScroll | (rSel << 3) | (displayEnable << 4) | (bitmapMode << 5) | (extendedColorMode << 6) | ((y >> 8) << 7);
        // Raster
        case 0x12:
            return y & 0x00FF;
        // Sprite enabled
        case 0x15: {
            uint8_t enabled = 0;
            for (int i = 0; i < 8; i++) {
                enabled |= sprites[i].spriteEnabled << i;
            }
            return enabled;
        }
        // Control register 2
        case 0x16:
            return xScroll | (cSel << 3) | (multiColorMode << 4) | 0xC0;
        // Sprite Y expansion
        case 0x17: {
            uint8_t yExp = 0;
            for (int i = 0; i < 8; i++) {
                yExp |= sprites[i].spriteYExpansion << i;
            }
            return yExp;
        }
        // Memory pointers
        case 0x18:
            return 0x1 | ((charGenMemoryPosition & 0x07) << 1) | ((videoMatrixMemoryPosition & 0x0F) << 4);
        // Interrupt register
        case 0x19:
            return (rasterInterrupt ? 0x01 : 0) | (spriteBitmapCollisionInterrupt ? 0x02 : 0) | (spriteSpriteCollisionInterrupt ? 0x04 : 0) | (lightpenInterrupt ? 0x08 : 0) | 0x70 | (IRQ ? 0x80 : 0);
        // Interrupt enabled
        case 0x1A:
            return (enableRasterInterrupt ? 0x01 : 0) | (enableSpriteBitmapCollisionInterrupt ? 0x02 : 0) | (enableSpriteSpriteCollisionInterrupt ? 0x04 : 0) | (enableLightpenInterrupt ? 0x08 : 0) | 0xF0;
        // Sprite data priority
        case 0x1B: {
            uint8_t dataPrio = 0;
            for (int i = 0; i < 8; i++) {
                dataPrio |= sprites[i].spriteDataPriority << i;
            }
            return dataPrio;
        }
        // Sprite multicolor
        case 0x1C: {
            uint8_t spriteMC = 0;
            for (int i = 0; i < 8; i++) {
                spriteMC |= sprites[i].spriteMulticolor << i;
            }
            return spriteMC;
        }
        // Sprite X expansion
        case 0x1D: {
            uint8_t xExp = 0;
            for (int i = 0; i < 8; i++) {
                xExp |= sprites[i].spriteXExpansion << i;
            }
            return xExp;
        }
        // Sprite-sprite collision
        case 0x1E: {
            uint8_t spriteSpriteColl = 0;
            for (int i = 0; i < 8; i++) {
                spriteSpriteColl |= sprites[i].spriteSpriteCollision << i;
                sprites[i].spriteSpriteCollision = 0;
            }
            return spriteSpriteColl;
        }
        // Sprite-data collision
        case 0x1F: {
            uint8_t spriteDataColl = 0;
            for (int i = 0; i < 8; i++) {
                spriteDataColl |= sprites[i].spriteDataCollision << i;
                sprites[i].spriteDataCollision = 0;
            }
            return spriteDataColl;
        }
        // Border color
        case 0x20:
            return borderColor | 0xF0;
        // Background color 0
        case 0x21:
            return backgroundColors[0] | 0xF0;
        // Backgorund color 1
        case 0x22:
            return backgroundColors[1] | 0xF0;
        // Backgorund color 2
        case 0x23:
            return backgroundColors[2] | 0xF0;
        // Backgorund color 3
        case 0x24:
            return backgroundColors[3] | 0xF0;
        // Sprite multicolor 0
        case 0x25:
            return spriteMulticolor0 | 0xF0;
        // Sprite multicolor 1
        case 0x26:
            return spriteMulticolor1 | 0xF0;
        // Color sprite 0
        case 0x027:
            return sprites[0].spriteColor | 0xF0;
        // Color sprite 1
        case 0x028:
            return sprites[1].spriteColor | 0xF0;
        // Color sprite 2
        case 0x029:
            return sprites[2].spriteColor | 0xF0;
        // Color sprite 3
        case 0x02A:
            return sprites[3].spriteColor | 0xF0;
        // Color sprite 4
        case 0x2B:
            return sprites[4].spriteColor | 0xF0;
        // Color sprite 5
        case 0x02C:
            return sprites[5].spriteColor | 0xF0;
        // Color sprite 6
        case 0x02D:
            return sprites[6].spriteColor | 0xF0;
        // Color sprite 7
        case 0x02E:
            return sprites[7].spriteColor | 0xF0;
        default:
            return 0xFF;
    }
}

void VIC::write(uint16_t addr, uint8_t data) {
    uint16_t effAddr = addr % 64;
    switch (effAddr) {
        // X coordinate sprite 0
        case 0x00:
            sprites[0].xCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // Y coordinate sprite 0
        case 0x01:
            sprites[0].yCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // X coordinate sprite 1
        case 0x02:
            sprites[1].xCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // Y coordinate sprite 1
        case 0x03:
            sprites[1].yCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // X coordinate sprite 2
        case 0x04:
            sprites[2].xCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // Y coordinate sprite 2
        case 0x05:
            sprites[2].yCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // X coordinate sprite 3
        case 0x06:
            sprites[3].xCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // Y coordinate sprite 3
        case 0x07:
            sprites[3].yCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // X coordinate sprite 4
        case 0x08:
            sprites[4].xCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // Y coordinate sprite 4
        case 0x09:
            sprites[4].yCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // X coordinate sprite 5
        case 0x0A:
            sprites[5].xCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // Y coordinate sprite 5
        case 0x0B:
            sprites[5].yCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // X coordinate sprite 6
        case 0x0C:
            sprites[6].xCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // Y coordinate sprite 6
        case 0x0D:
            sprites[6].yCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // X coordinate sprite 7
        case 0x0E:
            sprites[7].xCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        // Y coordinate sprite 7
        case 0x0F:
            sprites[7].yCoord = data | (sprites[0].xCoord & 0x0100);
            break;
        case 0x10:
            for (int i = 0; i < 8; i++) {
                sprites[i].xCoord = (sprites[i].xCoord & 0x00FF) | (((data >> i) & 0x01) << 8);
            }
            break;
        // Control register 1
        case 0x11:
            yScroll = data & 0x07;
            cSel = data & 0x08;
            displayEnable = data & 0x10;
            bitmapMode = data & 0x20;
            extendedColorMode = data & 0x40;
            rasterCompareLine |= (data >> 7) << 8;
            break;
        // Raster counter
        case 0x12:
            rasterCompareLine = data;
            break;
        // Sprite enabled
        case 0x15:
            for (int i = 0; i < 8; i++) {
                sprites[i].spriteEnabled = (data >> i) & 0x01;
            }
            break;
        // Control register 2
        case 0x16:
            xScroll = data & 0x07;
            cSel = data & 0x08;
            multiColorMode = data & 0x10;
            break;
        case 0x17:
            for (int i = 0; i < 8; i++) {
                sprites[i].spriteYExpansion = (data >> i) & 0x01;
            }
            break;
        // Memory pointers
       case 0x18:
            charGenMemoryPosition = (data & 0x0E) >> 1;
            videoMatrixMemoryPosition = (data & 0xF0) >> 4;
            break;
        // Interrupt register
        case 0x19:
            rasterInterrupt = data & 0x01;
            spriteBitmapCollisionInterrupt = data & 0x02;
            spriteSpriteCollisionInterrupt = data & 0x04;
            lightpenInterrupt = data & 0x08;
            IRQ = data & 0x80;
            break;
        // Interrupt enabled
        case 0x1A:
            enableRasterInterrupt = data & 0x01;
            enableSpriteBitmapCollisionInterrupt = data & 0x02;
            enableSpriteSpriteCollisionInterrupt = data & 0x04;
            enableLightpenInterrupt = data & 0x08;
            break;
        case 0x1B:
            for (int i = 0; i < 8; i++) {
                sprites[i].spriteDataPriority = (data >> i) & 0x01;
            }
            break;
        // Sprite multicolor
        case 0x1C:
            for (int i = 0; i < 8; i++) {
                sprites[i].spriteMulticolor = (data >> i) & 0x01;
            }
            break;
        // Sprite X epansion
        case 0x1D:
            for (int i = 0; i < 8; i++) {
                sprites[i].spriteXExpansion = (data >> i) & 0x01;
            }
            break;
        // Border color
        case 0x20:
            borderColor = data & 0x0F;
            break;
        // Background Color 0
        case 0x21:
            backgroundColors[0] = data & 0x0F;
            break;
        // Background Color 1
        case 0x22:
            backgroundColors[1] = data & 0x0F;
            break;
        // Background Color 2
        case 0x23:
            backgroundColors[2] = data & 0x0F;
            break;
        // Background Color 3
        case 0x24:
            backgroundColors[3] = data & 0x0F;
            break;
        // Sprite multicolor 0
        case 0x25:
            spriteMulticolor0 = data & 0x0F;
            break;
        // Sprite multicolor 1
        case 0x26:
            spriteMulticolor1 = data & 0x0F;
            break;
        // Color sprite 0
        case 0x27:
            sprites[0].spriteColor = data & 0x0F;
            break;
        // Color sprite 1
        case 0x28:
            sprites[1].spriteColor = data & 0x0F;
            break;
        // Color sprite 2
        case 0x29:
            sprites[2].spriteColor = data & 0x0F;
            break;
        // Color sprite 3
        case 0x2A:
            sprites[3].spriteColor = data & 0x0F;
            break;
        // Color sprite 4
        case 0x2B:
            sprites[4].spriteColor = data & 0x0F;
            break;
        // Color sprite 5
        case 0x2C:
            sprites[5].spriteColor = data & 0x0F;
            break;
        // Color sprite 6
        case 0x2D:
            sprites[6].spriteColor = data & 0x0F;
            break;
        // Color sprite 7
        case 0x2E:
            sprites[7].spriteColor = data & 0x0F;
            break;
        default:
            break;
    }
}

// Checks if interrupt enable register and interrupt latch are both set for any interrupt
// If so, IRQ is set to true
void VIC::checkIRQ() {
    if (enableRasterInterrupt & rasterInterrupt) IRQ = true; // RST
    if (enableSpriteBitmapCollisionInterrupt & spriteBitmapCollisionInterrupt) IRQ = true; // MBC
    if (enableSpriteSpriteCollisionInterrupt & spriteSpriteCollisionInterrupt) IRQ = true; // MMC
    if (enableLightpenInterrupt & lightpenInterrupt) IRQ = true; // LP
}
