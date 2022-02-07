#include "sprites.h"

#include "vic.h"

uint8_t Sprites::spritePAccess(int spriteNr) {
    return vic->accessMem((vic->videoMatrixMemoryPosition << 10)| 0x7F << 3 | spriteNr).val;
}

std::array<uint8_t, 8> Sprites::spriteSAccess(int spriteNr, uint8_t p) {
    auto s = vic->accessMem(p << 6 | spriteData[spriteNr].spriteDataCounter).val;

    std::array<uint8_t, 8> data = { 0 };
    if (spriteData[spriteNr].spriteMulticolor) {
        for (int i = 0; i < 4; i++) {
            if (((s >> (6 - 2 * i)) & 0x3) == 0x0) data[2*i] = data[2*i+1] = 0xFF;
            else if (((s >> (6 - 2 * i)) & 0x3) == 0x1) data[2*i] = data[2*i+1] = spriteMulticolor0;
            else if (((s >> (6 - 2 * i)) & 0x3) == 0x2) data[2*i] = data[2*i+1] = spriteData[spriteNr].spriteColor;
            else data[2*i] = data[2*i+1] = spriteMulticolor1;
        }
    }
    else {
        for (int i = 0; i < 8; i++) {
            data[i] = s & (1 << (7 - i)) ? spriteData[spriteNr].spriteColor : 0xFF;
        }
    }
    return data;
}
