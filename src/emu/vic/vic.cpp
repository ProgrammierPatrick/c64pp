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
        case 0x10:
            uint8_t msbs;
            for (int i = 0; i < 8; i++) {
                msbs |= ((sprites[i].xCoord >> 8) & 0x01) << i;
            }
            return msbs;
        // Control register 1
        case 0x11:
            return yScroll | (rSel << 3) | (displayEnable << 4) | (bitmapMode << 5) | (extendedColorMode << 6) | ((rasterCompareLine >> 8) << 7);
        // Raster
        case 0x12:
            return rasterCompareLine & 0x00FF;
        // Sprite enabled
        case 0x15:
            uint8_t enabled;
            for (int i = 0; i < 8; i++) {
                enabled |= sprites[i].spriteEnabled << i;
            }
            return enabled;
        // Control register 2
        case 0x16:
            return xScroll | (cSel << 3) | (multiColorMode << 4) | 0xC0;
        // Sprite Y expansion
        case 0x17:
            uint8_t yExp;
            for (int i = 0; i < 8; i++) {
                yExp |= sprites[i].spriteYExpansion << i;
            }
            return yExp;
        // Memory pointers
        case 0x18:
            return 0x1 | ((charGenMemoryPosition & 0x07) << 1) | ((videoMatrixMemoryPosition & 0x0F) << 4);
        // Interrupt register
        case 0x19:
            return rasterInterrupt | (spriteBitmapCollisionInterrupt << 1) | (spriteSpriteCollisionInterrupt << 2) | (lightpenInterrupt << 3) | 0x70 | (IRQ << 7);
        // Interrupt enabled
        case 0x1A:
            return enableRasterInterrupt | (enableSpriteBitmapCollisionInterrupt << 1) | (enableSpriteSpriteCollisionInterrupt << 2) | (enableLightpenInterrupt << 3) | 0xF0;
        // Sprite data priority
        case 0x1B:
            uint8_t dataPrio;
            for (int i = 0; i < 8; i++) {
                dataPrio |= sprites[i].spriteDataPriority << i;
            }
            return dataPrio;
        // Sprite multicolor
        case 0x1C:
            uint8_t spriteMC;
            for (int i = 0; i < 8; i++) {
                spriteMC |= sprites[i].spriteMulticolor << i;
            }
            return spriteMC;
        // Sprite X expansion
        case 0x1D:
            uint8_t xExp;
            for (int i = 0; i < 8; i++) {
                xExp |= sprites[i].spriteXExpansion << i;
            }
            return xExp;
        // Sprite-sprite collision
        case 0x1E:
            uint8_t spriteSpriteColl;
            for (int i = 0; i < 8; i++) {
                spriteSpriteColl |= sprites[i].spriteSpriteCollision << i;
                sprites[i].spriteSpriteCollision = 0;
            }
            return spriteSpriteColl;
        case 0x1F:
            uint8_t spriteDataColl;
            for (int i = 0; i < 8; i++) {
                spriteDataColl |= sprites[i].spriteDataCollision << i;
                sprites[i].spriteDataCollision = 0;
            }
            return spriteDataColl;
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
            charGenMemoryPosition = data & 0x0E;
            videoMatrixMemoryPosition = data & 0xF0;
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
