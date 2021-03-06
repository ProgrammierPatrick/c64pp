#pragma once

#include "colored_val.h"
#include "outputPixels.h"
#include "background_graphics.h"
#include "sprites.h"
#include "../mem/memory.h"

#include <array>
#include <iostream>
#include <vector>

class CIA;

class VIC {
public:
    // VIC-II (6569)

    VIC(Memory* mainRAM, Memory* charROM, Memory* colorRAM, CIA* cia)
        : mainRAM(mainRAM), charROM(charROM), colorRAM(colorRAM), cia(cia),
          backgroundGraphics(this), sprites(this) {
        screen.resize(screenWidth * screenHeight);
    }

    void tick();

    ColoredVal accessMem(uint16_t addr);

    bool isBadLine() {
        return y >= 0x30 && y <= 0xf7
                && (y & 0x7) == (yScroll & 0x7)
                && displayEnableSetInThisFrame;
    }

    uint8_t read(uint16_t addr, bool nonDestructive = false);
    void write(uint16_t addr, uint8_t data);

private:
    OutputPixels tickBackground();
    OutputPixels tickSprites();
    void tickBorder();

    void checkIRQ();
public:

    bool BA = true; // Bus Available: when false, MPU is stunned on the next read

    uint16_t x = 0x194; // "sprite coordinate system", internal
    uint16_t y = 0; // "raster line number", internal, same range as rasterCompareLine
    uint16_t cycleInLine = 1; // counts 8 pixels at a time
    uint16_t rasterCompareLine = 0; // rst8 (1 bit), raster (8 bit)

    uint8_t RC = 0; // 3-bit
    uint16_t VC = 0;     // 10-bit
    uint16_t VCBASE = 0; // 10-bit
    uint8_t VMLI = 0; // 6-bit video matrix line index
    bool inDisplayState = false;
    bool displayEnableSetInThisFrame = false;

    OutputPixels backgroundGraphicsLastPixels;

    bool mainBorderFlipFlop = false;
    bool verticalBorderFlipFlop = false;

    // dimensions: y ranges across all raster lines, including! VBlank
    // visible lines: screen including border
    // display window: area inside border
    static const uint16_t lastY = 311; // including border and vblank
    static const uint16_t firstVisibleY = 16; // including border
    static const uint16_t lastVisibleY = 299; // including border

    static const uint16_t firstCycleX = 404; // including border and hblank (from VIC-Article)
    static const uint16_t firstVisibleX = 480; // including border          (from VIC-Article)
    static const uint16_t lastVisibleX = 380; // including border           (from VIC-Article)
    static const uint16_t maxX = 0x1F7; // =503                             (from VIC-Article)
    static const uint16_t firstVisibleCycle = 11;
    static const uint16_t firstBackgroundGraphicsCycle = 16;
    static const uint16_t lastBackgroundGraphicsCycle = 55;
    static const uint16_t lastVisibleCycle = 60;
    static const uint16_t lastCycle = 63;

    static const uint16_t screenWidth = 404; // actually 403, but Qt requires 32-bit aligned lines, firstVisibleX -> maxX, 0 -> lastVisibleX
    static const uint16_t screenHeight = 284; // = lastVisibleY - firstVisibleY + 1

    std::vector<uint8_t> screen;
    std::array<ColoredVal, 40> videoMatrixLine;

    uint8_t borderColor = 0; // (EC) 4-bit color

    std::array<uint8_t, 4> backgroundColors = { 0 }; // (BxC)

    bool IRQ = false;
    bool rasterInterrupt                      = false; // (IRST)
    bool enableRasterInterrupt                = false; // (ERST)
    bool spriteBitmapCollisionInterrupt       = false; // (IMBC)
    bool enableSpriteBitmapCollisionInterrupt = false; // (EMBC)
    bool spriteSpriteCollisionInterrupt       = false; // (IMMC)
    bool enableSpriteSpriteCollisionInterrupt = false; // (EMMC)
    bool lightpenInterrupt                    = false; // (ILP)
    bool enableLightpenInterrupt              = false; // (ELP)

    bool cSel = true;
    bool rSel = true;

    uint8_t xScroll = 0; // 3-bit
    uint8_t yScroll = 3; // 3-bit

    bool displayEnable = true; // (DEN)

    bool extendedColorMode = false; // (ECM)
    bool bitmapMode = false;        // (BMM)
    bool multiColorMode = false;    // (MCM)

    uint8_t videoMatrixMemoryPosition = 1; // 4-bit (VM10-VM13)
    uint8_t charGenMemoryPosition = 2; // 3-bit (CB11-CB13)

    BackgroundGraphics backgroundGraphics;
    Sprites sprites;

    Memory* mainRAM;
    Memory* charROM;
    Memory* colorRAM;
    CIA* cia;
};
