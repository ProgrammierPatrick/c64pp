#pragma once

#include "../mem/memory.h"

#include <array>

struct ColoredVal {
    uint8_t val;
    uint8_t col;
    ColoredVal(uint8_t val = 0, uint8_t col = 0) : val(val), col(col) { }
};

class VIC {
public:
    void tick();


private:
    ColoredVal read(uint16_t addr) {
        uint8_t val;
        if (((bankSetting & 2) == 0) && addr >= 0x1000 && addr <= 0x1FFF)
            val = charROM->read(addr - 0x1000);
        else {
            auto absAddr = addr + bankSetting * 0x4000;
            val = mainRAM->read(absAddr);
        }
        return ColoredVal(val, colorRAM->read(0x03FF));
    }
    ColoredVal readVM(uint16_t addr) { read(addr + videoMatrixMemoryPosition * 0x400); }
    ColoredVal readCG(uint16_t addr) { read(addr + charGenMemoryPosition * 0x800); }

    bool isBadLine() {
        return y >= 0x30 && y <= 0xf7 && (y & 0x7) == (yScroll & 0x7) && denSetInThisLine;
    }

public:
    bool BA = true; // Bus Available: when true, MPU may use the bus and is not "stunned"

    uint16_t x;         // (RC), [RST8, RASTER]
    uint16_t y;         // (VC)

    uint8_t RC; // 3-bit
    uint16_t VC;     // 10-bit
    uint16_t VCBASE; // 10-bit
    uint8_t VMLI; // 6-bit video matrix line index
    bool inDisplayState = false;

    // uint16_t MOBDataCounter;     // (MC)

    const uint16_t firstX = 0x1E0;
    const uint16_t maxX = 0x1F7;
    const uint16_t lastX = 0x17C;
    const uint16_t screenWidth = 403; // maxX - firstX + 1 + lastX + 1;
    const uint16_t screenHeight = 284;

    std::vector<uint8_t> screen(screenWidth * screenHeight);
    std::array<ColoredVal, 40> videoMatrixLine;

    uint8_t borderColor; // (EC) 4-bit color

    std::array<uint8_t, 4> backgroundColors; // (BxC)

    bool cSel = false;
    bool rSel = false;
    bool denSetInThisLine = false; // WTF IS THIS

    uint8_t xScroll = 0; // 3-bit
    uint8_t yScroll = 3; // 3-bit

    bool extendedColorMode = false; // (ECM)
    bool bitmapMode = false;        // (BMM)
    bool multiColorMode = false;    // (MCM)

    uint8_t bankSetting;
    uint8_t videoMatrixMemoryPosition; // 4-bit
    uint8_t charGenMemoryPosition; // 4-bit

    Memory* mainRAM;
    Memory* charROM;
    Memory* colorRAM;
};
