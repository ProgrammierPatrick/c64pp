#include "sid.h"


void SID::process(size_t sampleCount, float* buffer) {
    for (int i = 0; i < sampleCount; i++)
        buffer[i] = 0.0f;

    // process voices
    for (int i = 0; i < 3; i++)
        tempVoiceBuffers[i].resize(sampleCount);

    float time = 0;
    float dt = phiRate / sampleRate;
    /*
    for (size_t sampleIdx = 0; sampleIdx < sampleCount; sampleIdx++) {
        int beforeTicks = time;
        time += dt;
        int afterTicks = time;
        for (size_t tick = 0; tick < afterTicks - beforeTicks; tick++)
            for (int v = 0; v < 3; v++)
                voices[v].tick();

        // for (int v = 0; v < 3; v++)
        //     tempVoiceBuffers[v][sampleIdx] = voices[v].getOutput();

        // running average filter
        const float alpha = 0.9f;
        float vol = volume * 1.0 / 0x0F;
        float mixed = vol * (1.0f / 3.0f) * (voices[0].getOutput() + voices[1].getOutput() + voices[2].getOutput());
        float aaMixed = alpha * mixed + alpha * aaTempValue;
        buffer[sampleIdx] = aaMixed;
        aaTempValue = aaMixed;
    }
    */

    // NONFILTERED, ALIASED VERSION
    /* for (size_t sampleIdx = 0; sampleIdx < sampleCount; sampleIdx++) {
        int beforeTicks = time;
        time += dt;
        int afterTicks = time;
        for (size_t tick = 0; tick < afterTicks - beforeTicks; tick++)
            for (int v = 0; v < 3; v++)
                voices[v].tick();
        float vol = volume * 1.0 / 0x0F;
        float mixed = vol * (1.0f / 3.0f) * (voices[0].getOutput() + voices[1].getOutput() + voices[2].getOutput());
        buffer[sampleIdx] = mixed;
    }*/

    // BOX FILTER
    for (size_t sampleIdx = 0; sampleIdx < sampleCount; sampleIdx++) {
        int beforeTicks = time;
        time += dt;
        int afterTicks = time;
        float sum = 0;
        for (int v = 0; v < 3; v++) {
            for (size_t tick = 0; tick < afterTicks - beforeTicks; tick++) {
                voices[v].tick();
                sum += voices[v].getOutput();
            }
        }
        float vol = volume * 1.0 / 0x0F;
        buffer[sampleIdx] = vol * (1.0f / (3 * (afterTicks - beforeTicks))) * sum;
    }

    // 2-Sample BOX FILTER
    /*for (size_t sampleIdx = 0; sampleIdx < sampleCount; sampleIdx++) {
        int beforeTicks = time;
        time += dt;
        int afterTicks = time;
        float sum = 0;
        for (int v = 0; v < 3; v++) {
            for (size_t tick = 0; tick < afterTicks - beforeTicks; tick++) {
                voices[v].tick();
                sum += voices[v].getOutput();
            }
        }
        float vol = volume * 1.0 / 0x0F;
        float result = vol * (1.0f / (3 * (afterTicks - beforeTicks))) * sum;
        buffer[sampleIdx] = 0.5f * (result + aaTempValue);
        aaTempValue = buffer[sampleIdx];
    }*/

    // 200-Tap FIR filter
    // pal freq: 985248.6 Hz
    // http://t-filter.engineerjs.com/
    // 0Hz - 20kHz: 5dB
    // 22.05kHz - 492624 Hz: -20dB
    // sampling freq: 985248 Hz, 200 taps
    /*float vol = volume * 1.0 / 0x0F;
    static float filter_taps[200] = {
      0.05590587714945222, -0.013813386741464024, -0.012505635511713353, -0.011506455946589332, -0.010767122700286591, -0.010234249324357094, -0.009864171407374523, -0.009622517479494275,
      -0.009474019227797396, -0.009390210482422043, -0.009350286687572852, -0.009331161313303771, -0.009310353435769868, -0.00927371813152143, -0.009210783729831171, -0.009108800895920154,
      -0.0089561765102643, -0.008746011256406097, -0.008474225031687143, -0.008137940993926262, -0.0077352498332730266, -0.0072643558364198935, -0.006725840686623048, -0.0061260347990286105,
      -0.005471912898976384, -0.00476526993557017, -0.004011030485580887, -0.0032229035195712884, -0.0024105886764028677, -0.001578034596706767, -0.0007393290081491614, 0.000091795772691555,
      0.0009015540409287855, 0.0017325062902047808, 0.0023754739522776707, 0.003171593695528047, 0.0038053158604099756, 0.004317547034680517, 0.004780097039208803, 0.005187241323438505,
      0.005500230078869078, 0.005694787091648525, 0.005764423224032368, 0.0057151176530835434, 0.005558604648805638, 0.005299987168393011, 0.004937171043714073, 0.004470769106290627, 0.0039049298470053874,
      0.003242484434623269, 0.0024881955282347276, 0.001654176532883245, 0.000757370749314493, -0.00018542629930287023, -0.0011584980083035404, -0.0021473059655922147, -0.0031373365460010266, -0.0041099782245649676, -0.0050454788690454415, -0.005931616377639096,
      -0.006756659168217185, -0.007496791113507761, -0.008130511450270527, -0.008651768060440046, -0.009044002352286196, -0.009283638791711643, -0.009363782418852334, -0.009293521372519941, -0.009001938616012603,
      -0.008567336953773062, -0.007932410361353871, -0.007083655686025643, -0.006051696326789867, -0.004843987653199458, -0.003449450408225351, -0.0018705824736545856, -0.0001231438379105709, 0.001775878834816324,
      0.0038110391908335404, 0.005969792987104724, 0.008239857793250076, 0.010601562115634407, 0.013028972757953785, 0.015498784782692994, 0.01799089802195308, 0.020481759653093148, 0.022944628617138033, 0.025355306581780902,
      0.027692276933867284, 0.029933674251144838, 0.032056688128353106, 0.03403572669644693, 0.03584494439187383, 0.037469841194681545, 0.03890363608127712, 0.04012543379511021, 0.041109854047709025,
      0.04186012046832339, 0.04237719185813812, 0.04262562963878989, 0.04262562963878989, 0.04237719185813812, 0.04186012046832339, 0.041109854047709025, 0.04012543379511021, 0.03890363608127712, 0.037469841194681545,
      0.03584494439187383, 0.03403572669644693, 0.032056688128353106, 0.029933674251144838, 0.027692276933867284, 0.025355306581780902, 0.022944628617138033, 0.020481759653093148, 0.01799089802195308,
      0.015498784782692994, 0.013028972757953785, 0.010601562115634407, 0.008239857793250076, 0.005969792987104724, 0.0038110391908335404, 0.001775878834816324, -0.0001231438379105709, -0.0018705824736545856,
      -0.003449450408225351, -0.004843987653199458, -0.006051696326789867, -0.007083655686025643, -0.007932410361353871, -0.008567336953773062, -0.009001938616012603, -0.009293521372519941, -0.009363782418852334, -0.009283638791711643,
      -0.009044002352286196, -0.008651768060440046, -0.008130511450270527, -0.007496791113507761, -0.006756659168217185, -0.005931616377639096, -0.0050454788690454415, -0.0041099782245649676, -0.0031373365460010266, -0.0021473059655922147,
      -0.0011584980083035404, -0.00018542629930287023, 0.000757370749314493, 0.001654176532883245, 0.0024881955282347276, 0.003242484434623269, 0.0039049298470053874, 0.004470769106290627, 0.004937171043714073, 0.005299987168393011,
      0.005558604648805638, 0.0057151176530835434, 0.005764423224032368, 0.005694787091648525, 0.005500230078869078, 0.005187241323438505, 0.004780097039208803, 0.004317547034680517, 0.0038053158604099756, 0.003171593695528047, 0.0023754739522776707,
      0.0017325062902047808, 0.0009015540409287855, 0.000091795772691555, -0.0007393290081491614, -0.001578034596706767, -0.0024105886764028677, -0.0032229035195712884, -0.004011030485580887, -0.00476526993557017, -0.005471912898976384, -0.0061260347990286105, -0.006725840686623048,
      -0.0072643558364198935, -0.0077352498332730266, -0.008137940993926262, -0.008474225031687143, -0.008746011256406097, -0.0089561765102643, -0.009108800895920154, -0.009210783729831171, -0.00927371813152143, -0.009310353435769868, -0.009331161313303771, -0.009350286687572852,
      -0.009390210482422043, -0.009474019227797396, -0.009622517479494275, -0.009864171407374523, -0.010234249324357094, -0.010767122700286591, -0.011506455946589332, -0.012505635511713353, -0.013813386741464024, 0.05590587714945222
    };
    for (size_t sampleIdx = 0; sampleIdx < sampleCount; sampleIdx++) {
        int beforeTicks = time;
        time += dt;
        int afterTicks = time;
        for (size_t tick = 0; tick < afterTicks - beforeTicks; tick++) {
            for (int i = 0; i < 3; i++) voices[i].tick();
            aaBuffer[aaIndex] = vol * (1.0f / 3.0f) * (voices[0].getOutput() + voices[1].getOutput() + voices[2].getOutput());
            aaIndex = aaIndex < aaBuffer.size() ? aaIndex + 1 : 0;
        }
        float result = 0;
        for(size_t i = 0; i < 200; i++) {
            result += filter_taps[200 - i] * aaBuffer[(aaIndex + i + 1) % 200];
        }
        buffer[sampleIdx] = result;
    }*/
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

    if (addr == 0x15) return cutoff;
    if (addr == 0x16) return cutoff >> 8;
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
            cutoff &= 0xFF00;
            cutoff |= data;
            break;
        case 0x16:
            cutoff &= 0x00FF;
            cutoff |= data << 8;
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
