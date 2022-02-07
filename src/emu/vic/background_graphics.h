#pragma once

#include "colored_val.h"

#include <array>
#include <cstdint>

class VIC;

class BackgroundGraphics {
public:
    BackgroundGraphics(VIC* vic) : vic(vic) { }

    void cAccess();
    std::array<uint8_t, 8> gAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    std::array<uint8_t, 8> idleStateGAccess(bool bitmapMode, bool multiColorMode, bool extendedColorMode);

    std::array<uint8_t, 8> standardTextModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    std::array<uint8_t, 8> multicolorTextModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    std::array<uint8_t, 8> standardBitMapModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    std::array<uint8_t, 8> multicolorBitMapModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    std::array<uint8_t, 8> ECMTextModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    std::array<uint8_t, 8> invalidTextModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    std::array<uint8_t, 8> invalidBitMapMode1GAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    std::array<uint8_t, 8> invalidBitMapMode2GAccess(ColoredVal c, uint16_t VC, uint8_t RC);

public:
    VIC *vic;
};
