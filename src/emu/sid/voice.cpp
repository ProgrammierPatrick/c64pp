#include "voice.h"

#include <iostream>
#include <array>
#include <cmath>

void Voice::tick() {
    bool reset = test || sync && syncMaster->oscPhase == 0x0080'0000;
    oscPhase = reset ? 0 : ((oscPhase + freq) & 0x00FF'FFFF);
    tickNoise();

    envelope.tick();
}

double Voice::getOutput() {
    uint16_t out = getWaveOutput();
    double envValue = envelope.counter / 255.0;

    // ANALOG DCA
    double vcaOut = envValue * (2 * out / static_cast<double>(0xFFF) - 1);

    return vcaOut;
}

uint16_t Voice::getWaveOutput() {
    uint16_t out = 0x0FFF;
    uint16_t sawwave = oscPhase >> 12;
    if (saw)      out &= sawwave;
    if (triangle) {
        bool invert = ringMod ? ((sawwave & 0x800) ^ (syncMaster->oscPhase & 0x0080'0000)) : (sawwave & 0x800);
        out &= (invert ? ~sawwave : sawwave) << 1;
    }
    if (pulse)    out &= sawwave > pw ? 0xFFF : 0;
    if (noise)    out &= getNoiseOutput();
    return out;
}

void Voice::tickNoise() {
    if (test) noiseLFSR = 0x7F'FFFF;
    // clock LFSR when bit 19 in osc goes high
    if (!test && !(noiseLastPhase & 0x0008'0000) && (oscPhase & 0x0008'0000)) {
        // taps at 17 and 22
        noiseLFSR = (noiseLFSR << 1) | (((noiseLFSR >> 17) & 1) ^ ((noiseLFSR >> 22) & 1));
    }

    noiseLastPhase = oscPhase;
}

uint16_t Voice::getNoiseOutput() {
    //bits 0, 2, 5, 9, 11, 14, 18, 20
    auto v = noiseLFSR;
    return (((v >> 0) & 1) << 4) | (((v >> 2) & 1) << 5) | (((v >> 5) & 1) << 6) | (((v >> 9) & 1) << 7)
         | (((v >>11) & 1) << 8) | (((v >>14) & 1) << 9) | (((v >>18) & 1) <<10) | (((v >>20) & 1) <<11);
}

