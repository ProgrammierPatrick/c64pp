#pragma once

#include "colored_val.h"
#include "outputPixels.h"

#include <array>
#include <cstdint>

class VIC;

class BackgroundGraphics {
public:
    BackgroundGraphics(VIC* vic) : vic(vic) { }

    void cAccess();
    OutputPixels gAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    OutputPixels idleStateGAccess(bool bitmapMode, bool multiColorMode, bool extendedColorMode);

    OutputPixels standardTextModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    OutputPixels multicolorTextModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    OutputPixels standardBitMapModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    OutputPixels multicolorBitMapModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    OutputPixels ECMTextModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    OutputPixels invalidTextModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    OutputPixels invalidBitMapMode1GAccess(ColoredVal c, uint16_t VC, uint8_t RC);
    OutputPixels invalidBitMapMode2GAccess(ColoredVal c, uint16_t VC, uint8_t RC);

public:
    VIC *vic;
};
