#include "background_graphics.h"

#include "vic.h"

#include <iostream>

void BackgroundGraphics::cAccess() {
    if (vic->extendedColorMode == false && vic->bitmapMode == false && vic->multiColorMode == false)
        standardTextModeCAccess();
    else std::cout << "BackgroundGraphics: unimplemented Mode " << vic->extendedColorMode << "/" << vic->bitmapMode << "/" << vic->multiColorMode << std::endl;
}

void BackgroundGraphics::gAccess() {
    if (vic->extendedColorMode == false && vic->bitmapMode == false && vic->multiColorMode == false)
        standardTextModeGAccess();
    else std::cout << "BackgroundGraphics: unimplemented Mode " << vic->extendedColorMode << "/" << vic->bitmapMode << "/" << vic->multiColorMode << std::endl;
}

void BackgroundGraphics::standardTextModeCAccess() {
    vic->videoMatrixLine[vic->VMLI] = vic->accessMem(((vic->videoMatrixMemoryPosition & 0xF) << 10) | vic->VC);
}

void BackgroundGraphics::standardTextModeGAccess() {
    auto c = vic->videoMatrixLine[vic->VMLI];
    auto g = vic->accessMem(((vic->charGenMemoryPosition & 0x7) << 11) | (c.val << 3) | vic->RC).val;
    vic->screen[vic->y * VIC::screenHeight + vic->x + 0] = (g & 0x80) ? c.col : vic->backgroundColors[0];
    vic->screen[vic->y * VIC::screenHeight + vic->x + 1] = (g & 0x40) ? c.col : vic->backgroundColors[0];
    vic->screen[vic->y * VIC::screenHeight + vic->x + 2] = (g & 0x20) ? c.col : vic->backgroundColors[0];
    vic->screen[vic->y * VIC::screenHeight + vic->x + 3] = (g & 0x10) ? c.col : vic->backgroundColors[0];
    vic->screen[vic->y * VIC::screenHeight + vic->x + 4] = (g & 0x08) ? c.col : vic->backgroundColors[0];
    vic->screen[vic->y * VIC::screenHeight + vic->x + 5] = (g & 0x04) ? c.col : vic->backgroundColors[0];
    vic->screen[vic->y * VIC::screenHeight + vic->x + 6] = (g & 0x02) ? c.col : vic->backgroundColors[0];
    vic->screen[vic->y * VIC::screenHeight + vic->x + 7] = (g & 0x01) ? c.col : vic->backgroundColors[0];
}

