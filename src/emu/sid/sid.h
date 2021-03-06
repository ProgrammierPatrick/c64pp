#pragma once

#include "voice.h"
#include "filter.h"

#include <memory>
#include <array>
#include <vector>
#include <cstdint>

class SID {
public:
    SID(float sampleRate, float phiRate);
    ~SID();

    void process(size_t sampleCount, float* buffer);

    uint8_t read(uint16_t addr, bool nonDestructive = false);
    void write(uint16_t addr, uint8_t data);

private:
    void writeVoice(uint16_t addr, uint8_t data, int voice);

public:
    std::array<Voice, 3> voices;
    Filter filter;

    uint16_t cutoff = 0; // 11 bit value: linear between 30Hz and 10kHz TODO: verify, denpending on used caps
    uint8_t resonance = 0; // 4 bit value: linear from no res to max res
    std::array<bool, 3> filterVoice = {false, false, false};
    bool filterExtInput = false;
    bool lowPass = false; // enable 12dB/oct low pass filter
    bool bandPass = false; // enable 12dB/oct band pass filter
    bool highPass = false; // enable 12dB/oct high pass filter
    bool disableVoice3 = false;
    uint8_t volume = 0xF; // 4 bit volume: linear from 0h->off to Fh->1

private:
    std::array<std::vector<float>, 3> tempVoiceBuffers;

    float sampleRate;
    float phiRate;
};
