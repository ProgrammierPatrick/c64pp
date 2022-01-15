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
    if (!vic->verticalBorderFlipFlop)
        for (int i = 0; i < 8; i++)
            vic->screen[vic->y * VIC::screenHeight + vic->x + i] = (g & 0x80) ? c.col : vic->backgroundColors[0];
    else
        for (int i = 0; i < 8; i++)
            vic->screen[vic->y * VIC::screenHeight + vic->x + i] = vic->backgroundColors[0];
}

