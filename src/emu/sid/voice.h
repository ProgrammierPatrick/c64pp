#pragma once

#include "envelope.h"

#include <cstdint>

class Voice {
public:
    Voice(Voice* syncMaster, float sampleRate, float phiFreq) : syncMaster(syncMaster), sampleRate(sampleRate), phiFreq(phiFreq) { }

    void tick();
    float getOutput();

    uint16_t getWaveOutput(); // get current internal oscillator output as 12bit number

    void setGate(bool gate) {
        envelope.setGate(gate);
    }

private:
    void tickNoise();
    uint16_t getNoiseOutput();

public:
    Envelope envelope;

    uint16_t freq = 0; // 16 bit frequency: freq * phi2_freq / 16777216
    double getFrequency() const { return freq * phiFreq / 16777216.0; }
    uint16_t pw = 0x800;   // 12 bit pulse width: (pw / 40.95) %

    bool sync = false; // oscillator hard sync
    bool ringMod = false; // ring modulation
    bool test = false; // reset and hold osc at zero

    bool triangle = false;
    bool saw = false;
    bool pulse = false;
    bool noise = false;

    uint32_t oscPhase = 0;   // 24 bit phase accumulator
    uint32_t noiseLFSR = 0x7F'FFFF;  // 23 bit fibonacci LFSR for noise
    uint32_t noiseLastPhase = 0;

    Voice *syncMaster;
    float sampleRate;   // sample rate in Hz
    float phiFreq;   // frequency of C64 base clock in Hz, different between PAL and NTSC
};
