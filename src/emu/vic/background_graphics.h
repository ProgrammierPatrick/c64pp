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

public:
    VIC *vic;
};
