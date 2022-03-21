#include "sid.h"


void SID::process(size_t sampleCount, double* buffer) {
    for (int i = 0; i < sampleCount; i++)
        buffer[i] = 0.0f;

    // process voices
    for (int i = 0; i < 3; i++) {
        tempVoiceBuffers[i].resize(sampleCount);
        voices[i].process(sampleCount, tempVoiceBuffers[i].data());
    }

    // mix audio
    double vol = volume * 1.0 / 0x0F;
    for (size_t i = 0; i < sampleCount; i++) {
        double sum = tempVoiceBuffers[0][i] + tempVoiceBuffers[1][i] + tempVoiceBuffers[2][i];
        buffer[i] = vol * (1.0 / 3.0) * sum;
    }
}

uint8_t SID::read(uint16_t addr, bool nonDestructive) {
    addr &= 0xFFCF;
    // TODO: osc3/random and env3
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
    addr &= 0xFFCF;
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
            cutoff |= data & 0x7;
            break;
        case 0x16:
            cutoff &= 0xF807;
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
