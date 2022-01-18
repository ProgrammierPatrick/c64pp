#pragma once

#include <array>
#include <cstdint>

class VIC;

class BackgroundGraphics {
public:
    BackgroundGraphics(VIC* vic) : vic(vic) { }

    void cAccess();
    std::array<uint8_t, 8> gAccess();

    std::array<uint8_t, 8> standardTextModeGAccess();
    std::array<uint8_t, 8> multicolorTextModeGAccess();
    std::array<uint8_t, 8> standardBitMapModeGAccess();
    std::array<uint8_t, 8> multicolorBitMapModeGAccess();
    std::array<uint8_t, 8> ECMTextModeGAccess();
    std::array<uint8_t, 8> invalidTextModeGAccess();
    std::array<uint8_t, 8> invalidBitMapMode1GAccess();
    std::array<uint8_t, 8> invalidBitMapMode2GAccess();

public:
    VIC *vic;
};
