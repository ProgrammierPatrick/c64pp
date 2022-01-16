#include "background_graphics.h"

#include "vic.h"

#include <iostream>

void BackgroundGraphics::cAccess() {
    vic->videoMatrixLine[vic->VMLI] = vic->accessMem(((vic->videoMatrixMemoryPosition & 0xF) << 10) | vic->VC);
}

std::array<uint8_t, 8> BackgroundGraphics::gAccess() {
    if (vic->extendedColorMode == false && vic->bitmapMode == false && vic->multiColorMode == false)
        return standardTextModeGAccess();
    std::cout << "BackgroundGraphics: unimplemented Mode " << vic->extendedColorMode << "/" << vic->bitmapMode << "/" << vic->multiColorMode << std::endl;
    return {0, 0, 0, 0, 0, 0, 0, 0};
}

std::array<uint8_t, 8> BackgroundGraphics::standardTextModeGAccess() {
    auto c = vic->videoMatrixLine[vic->VMLI];
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x7) << 11) | (c.val << 3) | vic->RC).val;
    std::array<uint8_t, 8> pixels = { 0 };
    for (int i = 0; i < 8; i++) {
        pixels[i] = g & (1 << (7 - i)) ? c.col : vic->backgroundColors[0];
    }
    return pixels;
}
