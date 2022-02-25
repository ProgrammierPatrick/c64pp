#pragma once

#include <array>
#include <cstdint>

struct OutputPixels {
    std::array<uint8_t, 8> pixels = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
    std::array<bool, 8> isForeground = { false,false,false,false,false,false,false,false };
    std::array<int8_t, 8> spriteNr = { -1,-1,-1,-1,-1,-1,-1,-1 };
};
