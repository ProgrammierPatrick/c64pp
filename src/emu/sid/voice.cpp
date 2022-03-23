#include "voice.h"

#include <iostream>
#include <array>
#include <cmath>

void Voice::process(size_t sampleCount, double *buffer) {
    double time = 0;
    double dt = phiFreq / sampleRate;
    for (size_t i = 0; i < sampleCount; i++) {
        int beforeTicks = time;
        time += dt;
        int afterTicks = time;
        buffer[i] = tick(afterTicks - beforeTicks);
    }
}

uint16_t Voice::getWaveOutput() {
    uint16_t out = 0x0FFF;
    uint16_t sawwave = oscPhase >> 12;
    if (saw)      out &= sawwave;
    if (triangle) out &= (sawwave & 0x800 ? ~sawwave : sawwave) << 1 | 0x001;
    if (pulse)    out &= sawwave > pw ? 0xFFF : 0;
    if (noise)    out &= getNoiseOutput();
    return out;
}

void Voice::tickNoise() {
    if (test) noiseLFSR = 0x7F'FFFF;
    // clock LFSR when bit 19 in osc goes high
    if ((oscPhase & 0x000F'FFFF) == 0x0008'0000) {
        // taps at 17 and 22
        noiseLFSR = test ? 0x7F'FFFF : ((noiseLFSR << 1) | (((noiseLFSR >> 17) & 1) ^ ((noiseLFSR >> 22) & 1)));
    }
}

uint16_t Voice::getNoiseOutput() {
    //bits 0, 2, 5, 9, 11, 14, 18, 20
    auto v = noiseLFSR;
    return (((v >> 0) & 1) << 4) | (((v >> 2) & 1) << 5) | (((v >> 5) & 1) << 6) | (((v >> 9) & 1) << 7)
         | (((v >>11) & 1) << 8) | (((v >>14) & 1) << 9) | (((v >>18) & 1) <<10) | (((v >>20) & 1) <<11);
}

double Voice::tick(int numSteps) {
    for (int i = 0; i < numSteps; i++) {
        oscPhase = test ? 0 : ((oscPhase + freq) & 0x00FF'FFFF);
        tickNoise();
    }

    uint16_t out = getWaveOutput();

    for(int i = 0; i < numSteps; i++)
        envelope.tick();
    double envValue = envelope.counter / 255.0;

    // ANALOG DCA
    double vcaOut = envValue * (2 * out / static_cast<double>(0xFFF) - 1);

    return vcaOut;
}
