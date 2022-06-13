#include "sid.h"

SID::SID(float sampleRate, float phiRate) :
    sampleRate(sampleRate), phiRate(phiRate),
    voices({{Voice(&voices[2], sampleRate, phiRate), Voice(&voices[0], sampleRate, phiRate), Voice(&voices[1], sampleRate, phiRate)}}),
    filter(sampleRate) {


}

SID::~SID() {

}

void SID::process(size_t sampleCount, float* buffer) {
    for (int i = 0; i < sampleCount; i++)
        buffer[i] = 0.0f;

    // process voices
    for (int i = 0; i < 3; i++)
        tempVoiceBuffers[i].resize(sampleCount);

    float time = 0;
    float dt = phiRate / sampleRate;

    float C = 30 + cutoff * 9070 / static_cast<float>((1<<11) - 1);
    float Q = this->resonance * (2.5f / 16) + 1;
    filter.setParams(C, Q);

    for (size_t sampleIdx = 0; sampleIdx < sampleCount; sampleIdx++) {

        // BOX FILTER
        int beforeTicks = time;
        time += dt;
        int afterTicks = time;
        float voicesToFilter = 0;
        float voicesNoFilter = 0;
        for (int v = 0; v < 3; v++) {
            for (size_t tick = 0; tick < afterTicks - beforeTicks; tick++) {
                voices[v].tick();
                if (!(v == 2 && disableVoice3)) {
                    if (filterVoice[v])
                        voicesToFilter += voices[v].getOutput();
                    else
                        voicesNoFilter += voices[v].getOutput();
                }
            }
        }

        // ANALOG STATE VARIABLE FILTER
        float voicesFiltered = 0;
        if (lowPass)  voicesFiltered += filter.lowPass(voicesToFilter);
        if (highPass) voicesFiltered += filter.highPass(voicesToFilter);
        if (bandPass) voicesFiltered += filter.bandPass(voicesToFilter);

        // ANALOG MIXER
        float vol = volume * 1.0 / 0x0F;
        buffer[sampleIdx] = vol * (1.0f / (3 * (afterTicks - beforeTicks))) * (voicesFiltered + voicesNoFilter);
    }
}

uint8_t SID::read(uint16_t addr, bool nonDestructive) {
    addr &= 0xF81F;
    if (addr == 0x1B)
        return static_cast<uint8_t>(voices[2].getWaveOutput() >> 4);
    else if (addr == 0x1C)
        return voices[2].envelope.counter;

    // Normally, all other cases would return zero.
    // Since nobody will probably use this, output internal SID values here for debugging
    // TODO: remove this in the end (in case something requires this)

    auto readVoice = [this](int i, auto a) -> uint8_t {
        if (a == 0x00) return voices[i].freq;
        if (a == 0x01) return voices[i].freq >> 8;
        if (a == 0x02) return voices[i].pw;
        if (a == 0x03) return voices[i].pw >> 8;
        if (a == 0x04) return (voices[i].envelope.gate ? 0x01 : 0x00)
                | (voices[i].sync ? 0x02 : 0x00)
                | (voices[i].ringMod ? 0x04 : 0x00)
                | (voices[i].test ? 0x08 : 0x00)
                | (voices[i].triangle ? 0x10 : 0x00)
                | (voices[i].saw ? 0x20 : 0x00)
                | (voices[i].pulse ? 0x40 : 0x00)
                | (voices[i].noise ? 0x80 : 0x00);
        if (a == 0x05) return voices[i].envelope.attack << 4 | voices[i].envelope.decay;
        if (a == 0x06) return voices[i].envelope.sustain << 4 | voices[i].envelope.release;
        return 0;
    };

    if (addr <= 0x06 && addr >= 0x00) return readVoice(0, addr);
    if (addr <= 0x0D && addr >= 0x07) return readVoice(1, addr - 0x07);
    if (addr <= 0x14 && addr >= 0x0E) return readVoice(2, addr - 0x0E);

    if (addr == 0x15) return (cutoff & 7) | 0xF8;
    if (addr == 0x16) return cutoff >> 3;
    if (addr == 0x17) return resonance << 4 | (filterExtInput ? 0x08 : 0) | (filterVoice[2] ? 0x04 : 0) | (filterVoice[1] ? 0x02 : 0) | (filterVoice[0] ? 0x01 : 0);
    if (addr == 0x18) return (disableVoice3 ? 0x80 : 0) | (highPass ? 0x40 : 0) | (bandPass ? 0x20 : 0) | (lowPass ? 0x10 : 0) | volume;

    return 0;
}


void SID::writeVoice(uint16_t addr, uint8_t data, int voiceIdx) {
    auto& voice = voices[voiceIdx];
    switch (addr) {
    case 0x00:
        voice.freq &= 0xFF00;
        voice.freq |= data;
        break;
    case 0x01:
        voice.freq &= 0x00FF;
        voice.freq |= data << 8;
        break;
    case 0x02:
        voice.pw &= 0xFF00;
        voice.pw |= data;
        break;
    case 0x03:
        voice.pw &= 0x00FF;
        voice.pw |= (data & 0x0F) << 8;
        break;
    case 0x04:
        voice.setGate(data & 0x01);
        voice.sync     = data & 0x02;
        voice.ringMod  = data & 0x04;
        voice.test     = data & 0x08;
        voice.triangle = data & 0x10;
        voice.saw      = data & 0x20;
        voice.pulse    = data & 0x40;
        voice.noise    = data & 0x80;
        break;
    case 0x05:
        voice.envelope.attack  = data >> 4;
        voice.envelope.decay   = data & 0x0F;
        break;
    case 0x06:
        voice.envelope.sustain = data >> 4;
        voice.envelope.release = data & 0x0F;
        break;
    }
}

void SID::write(uint16_t addr, uint8_t data) {
    addr &= 0xF81F;
    if (addr <= 0x06 && addr >= 0x00)
        writeVoice(addr - 0x00, data, 0);
    else if (addr <= 0x0D && addr >= 0x07)
        writeVoice(addr - 0x07, data, 1);
    else if (addr <= 0x14 && addr >= 0x0E)
        writeVoice(addr - 0x0E, data, 2);
    else {
        switch (addr) {
        case 0x15:
            cutoff &= 0xFFF8;
            cutoff |= data & 7;
            break;
        case 0x16:
            cutoff &= 0x0007;
            cutoff |= data << 3;
            break;
        case 0x17:
            resonance  = data >> 4;
            filterExtInput = data & 0x08;
            filterVoice[2] = data & 0x04;
            filterVoice[1] = data & 0x02;
            filterVoice[0] = data & 0x01;
            break;
        case 0x18:
            disableVoice3 = data & 0x80;
            highPass      = data & 0x40;
            bandPass      = data & 0x20;
            lowPass       = data & 0x10;
            volume = data & 0x0F;
            break;
        }
    }
}
