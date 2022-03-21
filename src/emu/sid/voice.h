#pragma once

#include "envelope.h"

#include <cstdint>

class Voice {
public:
    Voice(double sampleRate, double phiFreq) : sampleRate(sampleRate), phiFreq(phiFreq) { }

    void process(size_t sampleCount, double* buffer);

    void setGate(bool gate) {
        envelope.setGate(gate);
    }

private:
    // void processOSC(size_t sampleCount, double* buffer);
    double tick(int numSteps);


public:
    Envelope envelope;

    uint16_t freq = 0; // 16 bit frequency: freq * phi2_freq / 16777216
    double getFrequency() const { return freq * phiFreq / 16777216.0; }
    uint16_t pw = 0x800;   // 12 bit pulse width: (pw / 40.95) %

    // bool gate = false; // envelope input: true means key is held down
    bool sync = false; // oscillator hard sync. TODO: implement
    bool ringMod = false; // ring modulation. TODO: implement
    bool test = false; // reset and hold osc at zero

    bool triangle = false;
    bool saw = false;
    bool pulse = false;
    bool noise = false;

    //uint8_t attack = 0;  // 4 bit value: Refer to table in data sheet for durations
    //uint8_t decay = 0;   // 4 bit value: Refer to table in data sheet for durations
    //uint8_t sustain = 0; // 4 bit value: 0h->0 Fh->1
    //uint8_t release = 0; // 5 bit value: Refer to table in data sheet for durations

    uint32_t oscPhase = 0;   // 24 bit phase accumulator
    double envState = 0;     // 0-2 in ADR mode, 0-1 in R mode

    double sampleRate;   // sample rate in Hz
    double phiFreq;   // frequency of C64 base clock in Hz, different between PAL and NTSC
    //double oscCorePhase = 0; // cycles 0 to 1

};
