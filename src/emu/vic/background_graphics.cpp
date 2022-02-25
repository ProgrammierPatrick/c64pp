#include "background_graphics.h"

#include "vic.h"

#include <iostream>

void BackgroundGraphics::cAccess() {
    vic->videoMatrixLine[vic->VMLI] = vic->accessMem(((vic->videoMatrixMemoryPosition & 0xF) << 10) | vic->VC);
    // if ((vic->VMLI & 0x7) == 0) {
    //     std::cout << "c-access: line[" << toHexStr(vic->VMLI) << "] = *" << toHexStr(static_cast<uint16_t>(((vic->videoMatrixMemoryPosition & 0xF) << 10) | vic->VC))
    //               << " memPos at " << (int)vic->videoMatrixMemoryPosition << ", shifted: " << toHexStr(static_cast<uint16_t>((vic->videoMatrixMemoryPosition & 0xF) << 10)) << std::endl;
    // }
}

OutputPixels BackgroundGraphics::gAccess(ColoredVal c, uint16_t VC, uint8_t RC) {
    // auto c = vic->videoMatrixLine[vic->VMLI];
    if (!vic->extendedColorMode && !vic->bitmapMode && !vic->multiColorMode)
        return standardTextModeGAccess(c, VC, RC);
    if (!vic->extendedColorMode && !vic->bitmapMode &&  vic->multiColorMode)
        return multicolorTextModeGAccess(c, VC, RC);
    if (!vic->extendedColorMode &&  vic->bitmapMode && !vic->multiColorMode)
        return standardBitMapModeGAccess(c, VC, RC);
    if (!vic->extendedColorMode &&  vic->bitmapMode &&  vic->multiColorMode)
        return multicolorBitMapModeGAccess(c, VC, RC);
    if ( vic->extendedColorMode && !vic->bitmapMode && !vic->multiColorMode)
        return ECMTextModeGAccess(c, VC, RC);
    if ( vic->extendedColorMode && !vic->bitmapMode &&  vic->multiColorMode)
        return invalidTextModeGAccess(c, VC, RC);
    if ( vic->extendedColorMode &&  vic->bitmapMode && !vic->multiColorMode)
        return invalidBitMapMode1GAccess(c, VC, RC);
    if ( vic->extendedColorMode &&  vic->bitmapMode &&  vic->multiColorMode)
        return invalidBitMapMode2GAccess(c, VC, RC);
    return {0, 0, 0, 0, 0, 0, 0, 0}; // cannot reach
}

OutputPixels BackgroundGraphics::standardTextModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC) {
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x7) << 11) | (c.val << 3) | RC).val;

    OutputPixels output;
    for (int i = 0; i < 8; i++) {
        if(g & (1 << (7 - i))) {
            output.pixels[i] = c.col;
            output.isForeground[i] = true;
        } else {
            output.pixels[i] = vic->backgroundColors[0];
            output.isForeground[i] = false;
        }
    }
    return output;
}

OutputPixels BackgroundGraphics::multicolorTextModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC) {
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x7) << 11) | (c.val << 3) | RC).val;

    OutputPixels output;
    if ((c.col & 0x8) == 0) {
        for (int i = 0; i < 8; i++) {
            if (g & (1 << (7 - i))) {
                output.pixels[i] = (c.col & 0x7);
                output.isForeground[i] = true;
            } else {
                output.pixels[i] = vic->backgroundColors[0];
                output.isForeground[i] = false;
            }
        }
    }
    else {
        for (int i = 0; i < 4; i++) {
            if (((g >> (6 - 2 * i)) & 0x3) == 0x3) {
                output.pixels[2*i] = output.pixels[2*i+1] = c.col & 0x7;
                output.isForeground[2*i] = output.isForeground[2*i+1] = true;
            } else if (((g >> (6 - 2 * i)) & 0x3) == 0x2) {
                output.pixels[2*i] = output.pixels[2*i+1] = vic->backgroundColors[2];
                output.isForeground[2*i] = output.isForeground[2*i+1] = true;
            }
            else {
                output.pixels[2*i] = output.pixels[2*i+1] = vic->backgroundColors[(g >> (6 - 2 * i) & 0x3)];
                output.isForeground[2*i] = output.isForeground[2*i+1] = false;
            }
        }
    }
    return output;
}

OutputPixels BackgroundGraphics::standardBitMapModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC) {
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x4) << 11) | ((VC & 0x3FF) << 3) | RC).val;

    OutputPixels output;
    for (int i = 0; i < 8; i++) {
        if (g & (1 << (7 - i))) {
            output.pixels[i] = (c.val >> 4);
            output.isForeground[i] = true;
        } else {
            output.pixels[i] = (c.val & 0x0F);
            output.isForeground[i] = false;
        }
    }
    return output;
}

OutputPixels BackgroundGraphics::multicolorBitMapModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC) {
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x4) << 11) | ((VC & 0x3FF) << 3) | RC).val;

    OutputPixels output;
    for (int i = 0; i < 4; i++) {
        if (((g >> (6 - 2 * i)) & 0x3) == 0x0) {
            output.pixels[2*i] = output.pixels[2*i+1] = vic->backgroundColors[0];
            output.isForeground[2*i] = output.isForeground[2*i+1] = false;
        }
        if (((g >> (6 - 2 * i)) & 0x3) == 0x1) {
            output.pixels[2*i] = output.pixels[2*i+1] = (c.val >> 4);
            output.isForeground[2*i] = output.isForeground[2*i+1] = false;
        }
        if (((g >> (6 - 2 * i)) & 0x3) == 0x2) {
            output.pixels[2*i] = output.pixels[2*i+1] = (c.val & 0x0F);
            output.isForeground[2*i] = output.isForeground[2*i+1] = true;
        }
        if (((g >> (6 - 2 * i)) & 0x3) == 0x3) {
            output.pixels[2*i] = output.pixels[2*i+1] = c.col;
            output.isForeground[2*i] = output.isForeground[2*i+1] = true;
        }
    }
    return output;
}

OutputPixels BackgroundGraphics::ECMTextModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC) {
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x7) << 11) | (c.val & 0x3F) << 3 | RC).val;

    OutputPixels output;
    for (int i = 0; i < 8; i++) {
        if ((g & (1 << (7 - i))) == 0) {
            output.pixels[i] = vic->backgroundColors[(c.val >> 6) & 0x3];
            output.isForeground[i] = false;
        }
        else {
            output.pixels[i] = c.col;
            output.isForeground[i] = true;
        }
    }
    return output;
}

OutputPixels BackgroundGraphics::invalidTextModeGAccess(ColoredVal c, uint16_t VC, uint8_t RC) {
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x7) << 11) | (c.val & 0x3F) << 3 | RC).val;

    OutputPixels output;
    if ((c.col & 0x8) == 0) {
        for (int i = 0; i < 8; i++) {
            if ((g & (1 << (7 - i))) == 0) {
                output.isForeground[i] = false;
            }
            else {
                output.isForeground[i] = true;
            }
            output.pixels[i] = 0;
        }
    }
    else {
        for (int i = 0; i < 4; i++) {
            if ((g >> (6 - 2 * i)) == 0x0 || (g >> (6 - 2 * i)) == 0x1)
                output.pixels[2*i] = output.pixels[2*i+1] = 0;
                output.isForeground[2*i] = output.isForeground[2*i+1] = false;
            if ((g >> (6 - 2 * i)) == 0x2 || (g >> (6 - 2 * i)) == 0x3)
                output.pixels[2*i] = output.pixels[2*i+1] = 0;
                output.isForeground[2*i] = output.isForeground[2*i+1] = true;
        }
    }
    return output;
}

OutputPixels BackgroundGraphics::invalidBitMapMode1GAccess(ColoredVal c, uint16_t VC, uint8_t RC) {
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x4) << 11) | (VC & 0x33F) << 3 | RC).val;

    OutputPixels output;
    for (int i = 0; i < 8; i++) {
        if ((g & (1 << (7 - i))) == 0) {
            output.isForeground[i] = false;
        }
        else {
            output.isForeground[i] = true;
        }
        output.pixels[i] = 0;
    }
    return output;
}

OutputPixels BackgroundGraphics::invalidBitMapMode2GAccess(ColoredVal c, uint16_t VC, uint8_t RC) {
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x4) << 11) | (VC & 0x33F) << 3 | RC).val;

    OutputPixels output;
    for (int i = 0; i < 4; i++) {
        if ((g >> (6 - 2 * i)) == 0x0 || (g >> (6 - 2 * i)) == 0x1)
            output.pixels[2*i] = output.pixels[2*i+1] = 0;
            output.isForeground[2*i] = output.isForeground[2*i+1] = false;
        if ((g >> (6 - 2 * i)) == 0x2 || (g >> (6 - 2 * i)) == 0x3)
            output.pixels[2*i] = output.pixels[2*i+1] = 0;
            output.isForeground[2*i] = output.isForeground[2*i+1] = true;
    }
    return output;
}

OutputPixels BackgroundGraphics::idleStateGAccess(bool bitmapMode, bool multiColorMode, bool extendedColorMode) {
    auto g = vic->accessMem(extendedColorMode ? 0x39FF : 0xFFFF).val;

    OutputPixels output;
    if (!bitmapMode && !(multiColorMode && extendedColorMode)) {
        for (int i = 0; i < 8; i++)
            if ((g & (1 << (7 - i)))) {
                output.pixels[i] = 0;
                output.isForeground[i] = true;
            } else {
                output.pixels[i] = vic->backgroundColors[0];
                output.isForeground[i] = false;
            }
    }
    if ((!extendedColorMode && bitmapMode && !multiColorMode)
            || (extendedColorMode && !bitmapMode && multiColorMode)
            || (extendedColorMode &&  bitmapMode && !multiColorMode)) {
        for (int i = 0; i < 8; i++) {
            if ((g & (1 << (7 - i)))) {
                output.isForeground[i] = true;
            } else {
                output.isForeground[i] = false;
            }
            output.pixels[i] = 0;
        }
    }
    if (!extendedColorMode &&  bitmapMode &&  multiColorMode)
        for (int i = 0; i < 4; i++) {
            if ((g >> (6 - 2 * i)) == 0x0)
                output.pixels[2*i] = output.pixels[2*i+1] = vic->backgroundColors[0];
                output.isForeground[2*i] = output.isForeground[2*i+1] = false;
            if ((g >> (6 - 2 * i)) == 0x1)
                output.pixels[2*i] = output.pixels[2*i+1] = 0;
                output.isForeground[2*i] = output.isForeground[2*i+1] = false;
            if ((g >> (6 - 2 * i)) == 0x2 || (g >> (6 - 2 * i)) == 0x3)
                output.pixels[2*i] = output.pixels[2*i+1] = 0;
                output.isForeground[2*i] = output.isForeground[2*i+1] = true;
        }
    if (extendedColorMode && bitmapMode && multiColorMode)
        for (int i = 0; i < 4; i++) {
            if ((g >> (6 - 2 * i)) == 0x0 || (g >> (6 - 2 * i)) == 0x1)
                output.pixels[2*i] = output.pixels[2*i+1] = 0;
                output.isForeground[2*i] = output.isForeground[2*i+1] = false;
            if ((g >> (6 - 2 * i)) == 0x2 || (g >> (6 - 2 * i)) == 0x3)
                output.pixels[2*i] = output.pixels[2*i+1] = 0;
                output.isForeground[2*i] = output.isForeground[2*i+1] = true;
        }
    return output;
}
