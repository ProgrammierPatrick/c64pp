#include "vic.h"

#include "../io/cia.h"

void VIC::tick() {
    std::array<uint8_t, 8> pixels = { 0xFF };

    OutputPixels graphicPixels = tickBackground();
    OutputPixels spritePixels = tickSprites(graphicPixels.isForeground);

    pixels = graphicPixels.pixels;

    for (int i = 0; i < 8; i++) {
        auto spriteNr = spritePixels.spriteNr[i];
        if ((!graphicPixels.isForeground[i] || !sprites.spriteData[spriteNr].spriteDataPriority) && spritePixels.pixels[i] != 0xFF) {
            pixels[i] = spritePixels.pixels[i];

            // set MxD bits (sprite-data collision) in register $d01f
            if (graphicPixels.isForeground[i])
                sprites.spriteData[spriteNr].spriteDataCollision = true;
        }
    }

    for (int i = 0; i < 8; i++) {
        int sy = y - firstVisibleY;
        int sx = (cycleInLine - firstVisibleCycle) * 8 + i + 4;
        if (pixels[i] != 0xFF && sx >= 0 && sx < screenWidth && sy >= 0 && sy < screenHeight)
            screen[sy * screenWidth + sx] = pixels[i];
    }

    tickBorder();

    // compare y with rasterCompareLine
    if (y == 0 && cycleInLine == 2 || y != 0 && cycleInLine == 1) {
        if (y == rasterCompareLine) rasterInterrupt = true; // raster interrupt (RST)
    }

    x += 8;
    if (x > VIC::maxX) x = 4; // reset happens in half cycle, in full cycle, x is already at 4.
    cycleInLine++;

    if (cycleInLine == 64) {
        cycleInLine = 1;
        y++;
        if (y > lastY) {
            y = 0;
            // for(int i = 0; i < screenWidth * screenHeight; i++) screen[i] = 0;
        }
    }

    checkIRQ();
}

OutputPixels VIC::tickBackground() {
    bool badLine = isBadLine();
    OutputPixels gPixels;

    if (cycleInLine == 9) {
        VMLI = 0;
    }
    if (cycleInLine == 11) {
        VC = VCBASE;
    }
    BA = !(cycleInLine >= 13 && cycleInLine <= 54 && badLine);
    if (cycleInLine == 14 && inDisplayState && badLine) {
        RC = 0;
    }
    if (cycleInLine >= 16 && cycleInLine <= 55 && inDisplayState) {
        gPixels = backgroundGraphics.gAccess(videoMatrixLine[VMLI], VC, RC);
        // graphicsDataPipeline[0] = gPixels.pixels;
        // advanceGraphicsPipeline();

        VC++;
        VMLI++;
    } else {
        // idle state
        gPixels = backgroundGraphics.idleStateGAccess(bitmapMode, multiColorMode, extendedColorMode);
        // graphicsDataPipeline[0] = gPixels.pixels;
        // advanceGraphicsPipeline();
    }
    if (cycleInLine >= 15 && cycleInLine <= 54 && inDisplayState && badLine) {
        backgroundGraphics.cAccess();
    }
    if (cycleInLine == 58 && (!inDisplayState || RC == 7)) {
        VCBASE = VC;
    }
    if (cycleInLine == 58 && RC == 7) {
        inDisplayState = false;
    }
    if (cycleInLine == 58 && (RC < 7 || badLine)) {
        RC++;
    }

    if (y >= 0 && y <= 0x2F) {
        VCBASE = 0;
    }
    if (y == 0x30 && !displayEnable && cycleInLine == 1)
        displayEnableSetInThisFrame = false;
    if (y == 0x30 && displayEnable)
        displayEnableSetInThisFrame = true;

    if (badLine) inDisplayState = true;

    OutputPixels outPixels;

    for (int i = 0; i < 8; i++) {
        if (i < xScroll) {
            outPixels.pixels[i]       = backgroundGraphicsLastPixels.pixels[      i - xScroll + 8];
            outPixels.isForeground[i] = backgroundGraphicsLastPixels.isForeground[i - xScroll + 8];
        } else {
            outPixels.pixels[i]       = gPixels.pixels[      i - xScroll];
            outPixels.isForeground[i] = gPixels.isForeground[i - xScroll];
        }
    }
    backgroundGraphicsLastPixels = gPixels;

    return outPixels;
}

OutputPixels VIC::tickSprites(std::array<bool,8> isForeground) {
    OutputPixels outputPixels;

    uint8_t spriteDataPrioReg = read(0x1B, false);
    std::array<bool,8> pixelOccupied { false };

    //cond. 2 toggle y expansion FF at end of line
    if (cycleInLine == 55) {
        for (int i = 0; i < 8; i++) {
            if (sprites.spriteData[i].spriteYExpansion) {
                sprites.spriteData[i].expansionFlipFlop = !sprites.spriteData[i].expansionFlipFlop;
            }
        }
    }
    // cond. 3 beginning of sprite: enableDMA
    if (cycleInLine == 55 || cycleInLine == 56) {
        for (int i = 0; i < 8; i++) {
            if (sprites.spriteData[i].spriteEnabled
                 && (sprites.spriteData[i].yCoord & 0x00FF) == (y & 0x00FF)) {
                sprites.spriteData[i].enableDMA = true;
                sprites.spriteData[i].spriteDataCounterBase = 0;
                if (sprites.spriteData[i].spriteYExpansion) sprites.spriteData[i].expansionFlipFlop = true;
            }
        }
    }
    // cond. 4 beginning of sprite: currentlyDisplayed
    if (cycleInLine == 58) {
        for (int i = 0; i < 8; i++) {
            sprites.spriteData[i].spriteDataCounter = sprites.spriteData[i].spriteDataCounterBase;
            if (sprites.spriteData[i].enableDMA
                    && (sprites.spriteData[i].yCoord & 0x00FF) == (y & 0x00FF)) {
                sprites.spriteData[i].currentlyDisplayed = true;
            }
        }
    }
    // cond. 7 increment MCBASE only if expansionFF set
    if (cycleInLine == 15) {
        for (int i = 0; i < 8; i++) {
            if (sprites.spriteData[i].expansionFlipFlop) {
                sprites.spriteData[i].spriteDataCounterBase += 2;
            }
        }
    }
    // cond. 8 check for sprite end; increment MCBASE only if expansionFF set
    if (cycleInLine == 16) {
        for (int i = 0; i < 8; i++) {
            if (sprites.spriteData[i].expansionFlipFlop) {
                sprites.spriteData[i].spriteDataCounterBase++;
                if (sprites.spriteData[i].spriteDataCounterBase > 63) {
                    sprites.spriteData[i].currentlyDisplayed = false;
                    sprites.spriteData[i].enableDMA = false;
                }
            }
        }
    }

    if (cycleInLine == 55 && sprites.spriteData[0].spriteEnabled) BA = false;
    if (cycleInLine == 57 && sprites.spriteData[1].spriteEnabled) BA = false;
    if (cycleInLine == 59 && sprites.spriteData[2].spriteEnabled) BA = false;
    if (cycleInLine == 61 && sprites.spriteData[3].spriteEnabled) BA = false;
    if (cycleInLine == 63 && sprites.spriteData[4].spriteEnabled) BA = false;
    if (cycleInLine ==  2 && sprites.spriteData[5].spriteEnabled) BA = false;
    if (cycleInLine ==  4 && sprites.spriteData[6].spriteEnabled) BA = false;
    if (cycleInLine ==  6 && sprites.spriteData[7].spriteEnabled) BA = false;
    if (cycleInLine == 58 && !sprites.spriteData[1].spriteEnabled && !sprites.spriteData[2].spriteEnabled) BA = true;
    if (cycleInLine == 60 && !sprites.spriteData[2].spriteEnabled && !sprites.spriteData[3].spriteEnabled) BA = true;
    if (cycleInLine == 62 && !sprites.spriteData[3].spriteEnabled && !sprites.spriteData[4].spriteEnabled) BA = true;
    if (cycleInLine ==  1 && !sprites.spriteData[4].spriteEnabled && !sprites.spriteData[5].spriteEnabled) BA = true;
    if (cycleInLine ==  3 && !sprites.spriteData[5].spriteEnabled && !sprites.spriteData[6].spriteEnabled) BA = true;
    if (cycleInLine ==  5 && !sprites.spriteData[6].spriteEnabled && !sprites.spriteData[7].spriteEnabled) BA = true;
    if (cycleInLine ==  7 && !sprites.spriteData[7].spriteEnabled) BA = true;
    if (cycleInLine ==  9) BA = true;

    // every line
    for (int i = 0; i < 8; i++) {
        auto& sprite = sprites.spriteData[i];

        // cond. 1 when no expansion, always load new data
        if (!sprite.spriteYExpansion) sprite.expansionFlipFlop = true;

        // cond. 5 p- and s-Accesses
        if (sprite.enableDMA) {
            auto pCycle = (i >= 3) ? (2 * i - 5) : (i * 2 + 58);
            if (cycleInLine == pCycle) {
                sprites.spritePointer = sprites.spritePAccess(i);
                if (sprite.spriteEnabled) {
                    sprite.pixels[0] = sprites.spriteSAccess(i, sprites.spritePointer);
                    sprite.spriteDataCounter++;
                }
            }
            if (cycleInLine == pCycle + 1 && sprite.spriteEnabled) {
                sprite.pixels[1] = sprites.spriteSAccess(i, sprites.spritePointer);
                sprite.spriteDataCounter++;
                sprite.pixels[2] = sprites.spriteSAccess(i, sprites.spritePointer);
                sprite.spriteDataCounter++;
            }
        }

        // cond. 6 draw pixels
        if (sprite.currentlyDisplayed) {
            for (int j = 0; j < 8; j++) {
                int pixelX = x + j + 4 > maxX ? x + j + 4 - maxX - 1 : x + j + 4;
                if (pixelX == sprite.xCoord) {
                    sprite.isDrawingPixels = true;
                    sprite.xExpansionFF = false;
                }
                if (sprite.isDrawingPixels) {
                    auto pixelColor = sprite.pixels[sprite.drawIndexByte][sprite.drawIndexPixel];
                    if (pixelColor != 0xFF) {
                        // sprite sprite collision here
                        if (pixelOccupied[j]) {
                            sprites.spriteData[outputPixels.spriteNr[j]].spriteSpriteCollision = true;
                            sprites.spriteData[i].spriteSpriteCollision = true;
                        } else {
                            outputPixels.pixels[j] = pixelColor;
                            outputPixels.spriteNr[j] = i;
                            pixelOccupied[j] = true;
                        }
                    }
                    if (!sprite.spriteXExpansion || sprite.xExpansionFF) {
                        sprite.drawIndexPixel++;
                        if (sprite.drawIndexPixel >= 8) {
                            sprite.drawIndexPixel = 0;
                            sprite.drawIndexByte++;
                            if (sprite.drawIndexByte >= 3) {
                                sprite.drawIndexByte = 0;
                                sprite.isDrawingPixels = false;
                            }
                        }
                    }
                    sprite.xExpansionFF = !sprite.xExpansionFF;
                }
            }
        }
    }
    return outputPixels;
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
        if (mainBorderFlipFlop && sy >= 0 && sy < screenHeight && sx >= 0  && sx < screenWidth) {
            screen[sy * screenWidth + sx] = borderColor;
        }
    }
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
            return sprites.spriteData[0].xCoord & 0x00FF;
        // Y coordinate sprite 0
        case 0x01:
            return sprites.spriteData[0].yCoord & 0x00FF;
        // X coordinate sprite 1
        case 0x02:
            return sprites.spriteData[1].xCoord & 0x00FF;
        // Y coordinate sprite 1
        case 0x03:
            return sprites.spriteData[1].yCoord & 0x00FF;
        // X coordinate sprite 2
        case 0x04:
            return sprites.spriteData[2].xCoord & 0x00FF;
        // Y coordinate sprite 2
        case 0x05:
            return sprites.spriteData[2].yCoord & 0x00FF;
        // X coordinate sprite 3
        case 0x06:
            return sprites.spriteData[3].xCoord & 0x00FF;
        // Y coordinate sprite 3
        case 0x07:
            return sprites.spriteData[3].yCoord & 0x00FF;
        // X coordinate sprite 4
        case 0x08:
            return sprites.spriteData[4].xCoord & 0x00FF;
        // Y coordinate sprite 4
        case 0x09:
            return sprites.spriteData[4].yCoord & 0x00FF;
        // X coordinate sprite 5
        case 0x0A:
            return sprites.spriteData[5].xCoord & 0x00FF;
        // Y coordinate sprite 5
        case 0x0B:
            return sprites.spriteData[5].yCoord & 0x00FF;
        // X coordinate sprite 6
        case 0x0C:
            return sprites.spriteData[6].xCoord & 0x00FF;
        // Y coordinate sprite 6
        case 0x0D:
            return sprites.spriteData[6].yCoord & 0x00FF;
        // X coordinate sprite 7
        case 0x0E:
            return sprites.spriteData[7].xCoord & 0x00FF;
        // Y coordinate sprite 7
        case 0x0F:
            return sprites.spriteData[7].yCoord & 0x00FF;
        // MSBs of X coordinates
        case 0x10: {
            uint8_t msbs = 0;
            for (int i = 0; i < 8; i++) {
                msbs |= ((sprites.spriteData[i].xCoord >> 8) & 0x01) << i;
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
                enabled |= sprites.spriteData[i].spriteEnabled << i;
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
                yExp |= sprites.spriteData[i].spriteYExpansion << i;
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
                dataPrio |= sprites.spriteData[i].spriteDataPriority << i;
            }
            return dataPrio;
        }
        // Sprite multicolor
        case 0x1C: {
            uint8_t spriteMC = 0;
            for (int i = 0; i < 8; i++) {
                spriteMC |= sprites.spriteData[i].spriteMulticolor << i;
            }
            return spriteMC;
        }
        // Sprite X expansion
        case 0x1D: {
            uint8_t xExp = 0;
            for (int i = 0; i < 8; i++) {
                xExp |= sprites.spriteData[i].spriteXExpansion << i;
            }
            return xExp;
        }
        // Sprite-sprite collision
        case 0x1E: {
            uint8_t spriteSpriteColl = 0;
            for (int i = 0; i < 8; i++) {
                spriteSpriteColl |= sprites.spriteData[i].spriteSpriteCollision << i;
                if (!nonDestructive) sprites.spriteData[i].spriteSpriteCollision = false;
            }
            return spriteSpriteColl;
        }
        // Sprite-data collision
        case 0x1F: {
            uint8_t spriteDataColl = 0;
            for (int i = 0; i < 8; i++) {
                spriteDataColl |= sprites.spriteData[i].spriteDataCollision << i;
                if (!nonDestructive) sprites.spriteData[i].spriteDataCollision = false;
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
            return sprites.spriteMulticolor0 | 0xF0;
        // Sprite multicolor 1
        case 0x26:
            return sprites.spriteMulticolor1 | 0xF0;
        // Color sprite 0
        case 0x027:
            return sprites.spriteData[0].spriteColor | 0xF0;
        // Color sprite 1
        case 0x028:
            return sprites.spriteData[1].spriteColor | 0xF0;
        // Color sprite 2
        case 0x029:
            return sprites.spriteData[2].spriteColor | 0xF0;
        // Color sprite 3
        case 0x02A:
            return sprites.spriteData[3].spriteColor | 0xF0;
        // Color sprite 4
        case 0x2B:
            return sprites.spriteData[4].spriteColor | 0xF0;
        // Color sprite 5
        case 0x02C:
            return sprites.spriteData[5].spriteColor | 0xF0;
        // Color sprite 6
        case 0x02D:
            return sprites.spriteData[6].spriteColor | 0xF0;
        // Color sprite 7
        case 0x02E:
            return sprites.spriteData[7].spriteColor | 0xF0;
        default:
            return 0xFF;
    }
}

void VIC::write(uint16_t addr, uint8_t data) {
    uint16_t effAddr = addr % 64;
    switch (effAddr) {
        // X coordinate sprite 0
        case 0x00:
            sprites.spriteData[0].xCoord = data | (sprites.spriteData[0].xCoord & 0x0100);
            break;
        // Y coordinate sprite 0
        case 0x01:
            sprites.spriteData[0].yCoord = data | (sprites.spriteData[0].xCoord & 0x0100);
            break;
        // X coordinate sprite 1
        case 0x02:
            sprites.spriteData[1].xCoord = data | (sprites.spriteData[1].xCoord & 0x0100);
            break;
        // Y coordinate sprite 1
        case 0x03:
            sprites.spriteData[1].yCoord = data | (sprites.spriteData[1].xCoord & 0x0100);
            break;
        // X coordinate sprite 2
        case 0x04:
            sprites.spriteData[2].xCoord = data | (sprites.spriteData[2].xCoord & 0x0100);
            break;
        // Y coordinate sprite 2
        case 0x05:
            sprites.spriteData[2].yCoord = data | (sprites.spriteData[2].xCoord & 0x0100);
            break;
        // X coordinate sprite 3
        case 0x06:
            sprites.spriteData[3].xCoord = data | (sprites.spriteData[3].xCoord & 0x0100);
            break;
        // Y coordinate sprite 3
        case 0x07:
            sprites.spriteData[3].yCoord = data | (sprites.spriteData[3].xCoord & 0x0100);
            break;
        // X coordinate sprite 4
        case 0x08:
            sprites.spriteData[4].xCoord = data | (sprites.spriteData[4].xCoord & 0x0100);
            break;
        // Y coordinate sprite 4
        case 0x09:
            sprites.spriteData[4].yCoord = data | (sprites.spriteData[4].xCoord & 0x0100);
            break;
        // X coordinate sprite 5
        case 0x0A:
            sprites.spriteData[5].xCoord = data | (sprites.spriteData[5].xCoord & 0x0100);
            break;
        // Y coordinate sprite 5
        case 0x0B:
            sprites.spriteData[5].yCoord = data | (sprites.spriteData[5].xCoord & 0x0100);
            break;
        // X coordinate sprite 6
        case 0x0C:
            sprites.spriteData[6].xCoord = data | (sprites.spriteData[6].xCoord & 0x0100);
            break;
        // Y coordinate sprite 6
        case 0x0D:
            sprites.spriteData[6].yCoord = data | (sprites.spriteData[6].xCoord & 0x0100);
            break;
        // X coordinate sprite 7
        case 0x0E:
            sprites.spriteData[7].xCoord = data | (sprites.spriteData[7].xCoord & 0x0100);
            break;
        // Y coordinate sprite 7
        case 0x0F:
            sprites.spriteData[7].yCoord = data | (sprites.spriteData[7].xCoord & 0x0100);
            break;
        case 0x10:
            for (int i = 0; i < 8; i++) {
                sprites.spriteData[i].xCoord = (sprites.spriteData[i].xCoord & 0x00FF) | (((data >> i) & 0x01) << 8);
            }
            break;
        // Control register 1
        case 0x11:
            yScroll = data & 0x07;
            rSel = data & 0x08;
            displayEnable = data & 0x10;
            bitmapMode = data & 0x20;
            extendedColorMode = data & 0x40;
            rasterCompareLine &= ~0x100;
            rasterCompareLine |= (data >> 7) << 8;
            break;
        // Raster counter
        case 0x12:
            rasterCompareLine &= 0x100;
            rasterCompareLine |= data;
            break;
        // Sprite enabled
        case 0x15:
            for (int i = 0; i < 8; i++) {
                sprites.spriteData[i].spriteEnabled = (data >> i) & 0x01;
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
                sprites.spriteData[i].spriteYExpansion = (data >> i) & 0x01;
            }
            break;
        // Memory pointers
       case 0x18:
            charGenMemoryPosition = (data & 0x0E) >> 1;
            videoMatrixMemoryPosition = (data & 0xF0) >> 4;
            break;
        // Interrupt register
        case 0x19:
            if (data & 0x01) rasterInterrupt = false;
            if (data & 0x02) spriteBitmapCollisionInterrupt = false;
            if (data & 0x04) spriteSpriteCollisionInterrupt = false;
            if (data & 0x08) lightpenInterrupt = false;
            if (!(enableRasterInterrupt && rasterInterrupt
                  || enableSpriteBitmapCollisionInterrupt && spriteBitmapCollisionInterrupt
                  || enableSpriteSpriteCollisionInterrupt && spriteSpriteCollisionInterrupt))
                IRQ = false;
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
                sprites.spriteData[i].spriteDataPriority = (data >> i) & 0x01;
            }
            break;
        // Sprite multicolor
        case 0x1C:
            for (int i = 0; i < 8; i++) {
                sprites.spriteData[i].spriteMulticolor = (data >> i) & 0x01;
            }
            break;
        // Sprite X epansion
        case 0x1D:
            for (int i = 0; i < 8; i++) {
                sprites.spriteData[i].spriteXExpansion = (data >> i) & 0x01;
            }
            break;
        // registers $d01e and $d01f cannot be written
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
            sprites.spriteMulticolor0 = data & 0x0F;
            break;
        // Sprite multicolor 1
        case 0x26:
            sprites.spriteMulticolor1 = data & 0x0F;
            break;
        // Color sprite 0
        case 0x27:
            sprites.spriteData[0].spriteColor = data & 0x0F;
            break;
        // Color sprite 1
        case 0x28:
            sprites.spriteData[1].spriteColor = data & 0x0F;
            break;
        // Color sprite 2
        case 0x29:
            sprites.spriteData[2].spriteColor = data & 0x0F;
            break;
        // Color sprite 3
        case 0x2A:
            sprites.spriteData[3].spriteColor = data & 0x0F;
            break;
        // Color sprite 4
        case 0x2B:
            sprites.spriteData[4].spriteColor = data & 0x0F;
            break;
        // Color sprite 5
        case 0x2C:
            sprites.spriteData[5].spriteColor = data & 0x0F;
            break;
        // Color sprite 6
        case 0x2D:
            sprites.spriteData[6].spriteColor = data & 0x0F;
            break;
        // Color sprite 7
        case 0x2E:
            sprites.spriteData[7].spriteColor = data & 0x0F;
            break;
    }
}

// Checks if interrupt enable register and interrupt latch are both set for any interrupt
// If so, IRQ is set to true
void VIC::checkIRQ() {
    if (enableRasterInterrupt && rasterInterrupt) IRQ = true; // RST
    if (enableSpriteBitmapCollisionInterrupt && spriteBitmapCollisionInterrupt) IRQ = true; // MBC
    if (enableSpriteSpriteCollisionInterrupt && spriteSpriteCollisionInterrupt) IRQ = true; // MMC
    if (enableLightpenInterrupt && lightpenInterrupt) IRQ = true; // LP
}
