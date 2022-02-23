#pragma once

#include <array>
#include <cstdint>

struct SpriteData {
    uint16_t xCoord = 0;
    uint16_t yCoord = 0;
    uint8_t spriteColor = 0;
    bool spriteEnabled = false;
    bool spriteXExpansion = false;
    bool spriteYExpansion = false;
    bool spriteSpriteCollision = false;
    bool spriteDataCollision = false;
    bool spriteDataPriority = false;
    bool spriteMulticolor = false;
    uint8_t spriteDataCounter = 0; // 6-bit "MC" MOB Data Counter (MC0-MC7?)
    uint8_t spriteDataCounterBase = 0; // 6-bit "MCBASE" MOB Data Counter Base
    bool expansionFlipFlop = true;
    bool currentlyDisplayed = false; // internal register
    bool enableDMA = false;
    std::array<std::array<uint8_t,8>, 3> pixels { 0 };

    // custom internals
    int drawIndexByte = 0;
    int drawIndexPixel = 0;
    bool xExpansionFF = false;
    bool isDrawingPixels = false;
};

class VIC;

class Sprites {
public:
    Sprites(VIC *vic) : vic(vic) { }

    uint8_t spritePAccess(int spriteNr);
    std::array<uint8_t, 8> spriteSAccess(int spriteNr, uint8_t p);

    VIC *vic;
    std::array<SpriteData, 8> spriteData;
    uint8_t spriteMulticolor0 = 0;
    uint8_t spriteMulticolor1 = 0;

    uint8_t spritePointer = 0; // result of p-Access. Only used in the 3 folling s-Accesses
};
