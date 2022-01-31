#include "background_graphics.h"

#include "vic.h"

#include <iostream>
#include "../../gui/text_utils.h"

void BackgroundGraphics::cAccess() {
    vic->videoMatrixLine[vic->VMLI] = vic->accessMem(((vic->videoMatrixMemoryPosition & 0xF) << 10) | vic->VC);
    // if ((vic->VMLI & 0x7) == 0) {
    //     std::cout << "c-access: line[" << toHexStr(vic->VMLI) << "] = *" << toHexStr(static_cast<uint16_t>(((vic->videoMatrixMemoryPosition & 0xF) << 10) | vic->VC))
    //               << " memPos at " << (int)vic->videoMatrixMemoryPosition << ", shifted: " << toHexStr(static_cast<uint16_t>((vic->videoMatrixMemoryPosition & 0xF) << 10)) << std::endl;
    // }
}

std::array<uint8_t, 8> BackgroundGraphics::gAccess() {
    if (!vic->extendedColorMode && !vic->bitmapMode && !vic->multiColorMode)
        return standardTextModeGAccess();
    if (!vic->extendedColorMode && !vic->bitmapMode &&  vic->multiColorMode)
        return multicolorTextModeGAccess();
    if (!vic->extendedColorMode &&  vic->bitmapMode && !vic->multiColorMode)
        return standardBitMapModeGAccess();
    if (!vic->extendedColorMode &&  vic->bitmapMode &&  vic->multiColorMode)
        return multicolorBitMapModeGAccess();
    if ( vic->extendedColorMode && !vic->bitmapMode && !vic->multiColorMode)
        return ECMTextModeGAccess();
    if ( vic->extendedColorMode && !vic->bitmapMode &&  vic->multiColorMode)
        return invalidTextModeGAccess();
    if ( vic->extendedColorMode &&  vic->bitmapMode && !vic->multiColorMode)
        return invalidBitMapMode1GAccess();
    if ( vic->extendedColorMode &&  vic->bitmapMode &&  vic->multiColorMode)
        return invalidBitMapMode2GAccess();
    return {0, 0, 0, 0, 0, 0, 0, 0}; // cannot reach
}

std::array<uint8_t, 8> BackgroundGraphics::standardTextModeGAccess() {
    auto c = vic->videoMatrixLine[vic->VMLI];
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x7) << 11) | 0x1000 | (c.val << 3) | vic->RC).val;

    std::array<uint8_t, 8> pixels = { 0 };
    for (int i = 0; i < 8; i++) {
        pixels[i] = g & (1 << (7 - i)) ? c.col : vic->backgroundColors[0];
    }
    return pixels;
}

std::array<uint8_t, 8> BackgroundGraphics::multicolorTextModeGAccess() {
    auto c = vic->videoMatrixLine[vic->VMLI];
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x7) << 11) | 0x1000 | (c.val << 3) | vic->RC).val;

    std::array<uint8_t, 8> pixels = { 0 };
    if ((c.col & 0x8) == 0) {
        for (int i = 0; i < 8; i++) {
            pixels[i] = g & (1 << (7 - i)) ? (c.col & 0x7) : vic->backgroundColors[0];
        }
    }
    else {
        for (int i = 0; i < 4; i++) {
            if (((g >> (6 - 2 * i)) & 0x3) == 0x3) pixels[2*i] = pixels[2*i+1] = c.col & 0x7;
            else pixels[2*i] = pixels[2*i+1] = vic->backgroundColors[(g >> (6 - 2 * i) & 0x3)];
        }
    }
    return pixels;
}

std::array<uint8_t, 8> BackgroundGraphics::standardBitMapModeGAccess() {
    auto c = vic->videoMatrixLine[vic->VMLI];
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x4) << 11) | 0x1000 | ((vic->VC & 0x3FF) << 3) | vic->RC).val;

    std::array<uint8_t, 8> pixels = { 0 };
    for (int i = 0; i < 8; i++) {
        pixels[i] = g & (1 << (7 - i)) ? (c.val >> 4) : (c.val & 0x0F);
    }
    return pixels;
}

std::array<uint8_t, 8> BackgroundGraphics::multicolorBitMapModeGAccess() {
    auto c = vic->videoMatrixLine[vic->VMLI];
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x4) << 11) | 0x1000 | ((vic->VC & 0x3FF) << 3) | vic->RC).val;

    std::array<uint8_t, 8> pixels = { 0 };
    for (int i = 0; i < 4; i++) {
        if (((g >> (6 - 2 * i)) & 0x3) == 0x0) pixels[2*i] = pixels[2*i+1] = vic->backgroundColors[0];
        if (((g >> (6 - 2 * i)) & 0x3) == 0x1) pixels[2*i] = pixels[2*i+1] = (c.val >> 4);
        if (((g >> (6 - 2 * i)) & 0x3) == 0x2) pixels[2*i] = pixels[2*i+1] = (c.val & 0x0F);
        if (((g >> (6 - 2 * i)) & 0x3) == 0x3) pixels[2*i] = pixels[2*i+1] = c.col;
    }
    return pixels;
}

std::array<uint8_t, 8> BackgroundGraphics::ECMTextModeGAccess() {
    auto c = vic->videoMatrixLine[vic->VMLI];
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x7) << 11) | 0x1000 | (c.val & 0x3F) << 3 | vic->RC).val;

    std::array<uint8_t, 8> pixels = { 0 };
    for (int i = 0; i < 8; i++) {
        if ((g & (1 << (7 - i))) == 0) {
            pixels[i] = vic->backgroundColors[(c.val >> 6) & 0x3];
        }
        else {
            pixels[i] = c.col;
        }
    }
    return pixels;
}

std::array<uint8_t, 8> BackgroundGraphics::invalidTextModeGAccess() {
    auto c = vic->videoMatrixLine[vic->VMLI];
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x7) << 11) | 0x1000 | (c.val & 0x3F) << 3 | vic->RC).val;

    std::array<uint8_t, 8> pixels = { 0 };
    if ((c.col & 0x8) == 0) {
        for (int i = 0; i < 8; i++) {
            pixels[i] = 0; // todo: differentiate between fore- and background
        }
    }
    else {
        for (int i = 0; i < 4; i++) {
            if ((g >> (6 - 2 * i)) == 0x0 || (g >> (6 - 2 * i)) == 0x1) pixels[2*i] = pixels[2*i+1] = 0; // todo: background
            if ((g >> (6 - 2 * i)) == 0x2 || (g >> (6 - 2 * i)) == 0x3) pixels[2*i] = pixels[2*i+1] = 0; // todo: foreground
        }
    }
    return pixels;
}

std::array<uint8_t, 8> BackgroundGraphics::invalidBitMapMode1GAccess() {
    auto c = vic->videoMatrixLine[vic->VMLI];
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x4) << 11) | 0x1000 | (vic->VC & 0x33F) << 3 | vic->RC).val;

    std::array<uint8_t, 8> pixels = { 0 };
    for (int i = 0; i < 8; i++) {
        pixels[i] = 0; // todo: differentiate between fore- and background
    }
    return pixels;
}

std::array<uint8_t, 8> BackgroundGraphics::invalidBitMapMode2GAccess() {
    auto c = vic->videoMatrixLine[vic->VMLI];
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x4) << 11) | 0x1000 | (vic->VC & 0x33F) << 3 | vic->RC).val;

    std::array<uint8_t, 8> pixels = { 0 };
    for (int i = 0; i < 4; i++) {
        if ((g >> (6 - 2 * i)) == 0x0 || (g >> (6 - 2 * i)) == 0x1) pixels[2*i] = pixels[2*i+1] = 0; // todo: background
        if ((g >> (6 - 2 * i)) == 0x2 || (g >> (6 - 2 * i)) == 0x3) pixels[2*i] = pixels[2*i+1] = 0; // todo: foreground
    }
    return pixels;
}
