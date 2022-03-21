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

const std::array<double,16> envTimeLin = {
    0.002, 0.008, 0.016, 0.024,
    0.038, 0.056, 0.068, 0.08,
    0.1,   0.25,  0.5,   0.8,
    1.0,   3.0,   5.0,   8.0
};
const std::array<double,16> envTimeExp = {
    0.006, 0.024, 0.048, 0.072,
    0.114, 0.168, 0.204, 0.24,
    0.3,   0.75,  1.5,   2.4,
    3.0,   9.0,   15.0,  24.0
};

double Voice::tick(int numSteps) {
    // --- oscillator
    oscPhase = (oscPhase + numSteps * freq) & 0x00FF'FFFF;
    uint16_t sawwave = oscPhase >> 12;

    uint16_t out = 0x0FFF;
    if (saw)      out &= sawwave;
    if (triangle) out &= (sawwave & 0x800 ? ~sawwave : sawwave) << 1 | 0x001;
    if (pulse)    out &= sawwave > pw ? 0xFFF : 0;
    // TODO: noise
    // The Noise waveform was created using a 23-bit pseudo-random sequence generator (i.e., a shift register with specific outputs fed back to the input through combinatorial logic).The shift register was clocked by one of the intermediate bits of the accumulator to keep the frequency content of the noise waveform relatively the same as the pitched waveforms. The upper 12-bits of the shift register were sent to the Waveform D/A.

    // ---- envelope generator
    /*double envValue = 0;
    if (envState < 2.0 && (gate || envState < 1.0)) {
        const double gamma = 1.3; // fake exp factor
        if (gate) {
            double dt = numSteps / phiFreq; // time in s
            if (envState < 1.0) {
                double dx = dt / envTimeLin[attack];
                envState = envState - dx;
                if (envState > 1.0) {
                    double dxLeft = std::max(0.0, envState - 1.0);
                    double dtLeft = dxLeft * envTimeLin[attack];
                    double dx = dtLeft / envTimeExp[decay];
                    envState = std::min(2.0, 1 - pow(log(1) / log(gamma) + dx, gamma) + 1);
                }
            } else {
                double dx = dt / envTimeExp[decay];
                envState = std::min(2.0, 1 - pow(log(1 - envState - 1) / log(gamma) + dx, gamma) + 1);
            }
            envValue = std::min(sustain / 15.0, 1 - std::abs(envState - 1.0));
        } else {
            double dt = numSteps / phiFreq; // time in s
            double dx = dt / envTimeExp[release];
            envState = std::min(1.0, 1 - pow(log(1 - envState) / log(gamma) + dx, gamma));
            envValue = 2.0 - envState;
        }
    }
    if (gate && envState >= 2.0)
        envValue = sustain / 15.0;
    */
    for(int i = 0; i < numSteps - 1; i++)
        envelope.tick();
    double envValue = envelope.tick() / 255.0;

    return envValue * (2 * out / static_cast<double>(0xFFF) - 1);
}

/*
// based on http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
void Voice::processOSC(size_t sampleCount, double* buffer) {
    if (freq == 0) {
        for (size_t i = 0; i < sampleCount; i++) buffer[i] = 0;
        return;
    }

    double dt = getFrequency() / sampleRate;

    for (size_t i = 0; i < sampleCount; i++) {
        // ------ calculate poly blep for anti aliasing
        // +1              /
        //  0   ,---------´
        // -1  /
        double polyBLEP = 0;
        if (oscCorePhase < dt) {
            double a = oscCorePhase / dt;
            polyBLEP = - a * a + 2 * a  - 1.0;
        } else if (oscCorePhase > 1.0 - dt) {
            double a = (oscCorePhase - 1) / dt;
            polyBLEP = a * a + 2 * a + 1.0;
        }

        // ------ waveforms
        double saw = 2.0 * oscCorePhase - 1.0;
        double sawAA = saw - polyBLEP;

        double tri = 2.0 * std::abs(2.0 * oscCorePhase - 1.0) - 1.0;

        double result = 0;
        if (this->triangle) result += tri;
        if (this->saw)      result += sawAA;
        if (this->pulse)    result += tri; // TODO: implement
        if (this->noise)    result += 0;   // TODO: implement

        buffer[i] = result;

        oscCorePhase = std::fmod(oscCorePhase + dt, 1.0);
    }
}
*/
