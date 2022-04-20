#include "testsid.h"

#include "../emu/sid/sid.h"
#include "../emu/text_utils.h"

#include <iostream>
#include <fstream>
#include <cmath>

std::string prompt(const std::string& name, const std::string& def) {
    std::cout << name << " [" << def << "] > ";
    std::string param = "";
    std::getline(std::cin, param);
    if (param == "") param = def;
    return param;
}

int envelopeTest() {
    std::cout << "run SID test: envelope\n";

    const int sampleRate = 44'100;
    const int phiRate = 985'248;
    SID sid(sampleRate, phiRate);
    sid.voices[1].test = true;
    sid.voices[2].test = true;
    sid.volume = 0xF;
    sid.voices[0].freq = 7382; // 440Hz at 1MHz, 7382*985248/16777216=443,511Hz
    sid.voices[0].saw = true;

    std::string outpath = prompt("output path", "test.bin");
    std::string param = prompt("sweep parameter (A,D,S,R)", "A");
    if (param[0] >= 'a' && param[0] <= 'z') param[0] -= ('a' - 'A');
    if (!(param == "A" || param == "D" || param == "S" || param == "R")) {
        std::cout << "invalid param selected. exiting.";
        return 1;
    }
    if (param != "A") {
        std::string in = prompt("value for A", "A");
        in = "0"+in;
        sid.voices[0].envelope.attack = fromHexStr8(in);
    }
    if (param != "D") {
        std::string in = prompt("value for D", "A");
        in = "0"+in;
        sid.voices[0].envelope.decay = fromHexStr8(in);
    }
    if (param != "S") {
        std::string in = prompt("value for S", "A");
        in = "0"+in;
        sid.voices[0].envelope.sustain = fromHexStr8(in);
    }
    if (param != "R") {
        std::string in = prompt("value for R", "A");
        in = "0"+in;
        sid.voices[0].envelope.release = fromHexStr8(in);
    }

    std::fstream file(outpath, std::ios::out | std::ios::binary);

    auto nextSample = [&file, &sid]() {
        float value;
        float sample;
        sid.process(1, &sample);
        value = sid.voices[0].envelope.counter / static_cast<float>(255);
        //value = sid.voices[0].envelope.countingEnabled ? 1 : 0;
        //value = sid.voices[0].envelope.debugLFSR15State * 1.0f / 0xFFFF;

        file.write(reinterpret_cast<char*>(&value), sizeof(float));
        file.write(reinterpret_cast<char*>(&sample), sizeof(float));
    };

    for(int i = 0; i <= 0xF; i++) {
        if (param == "A") sid.voices[0].envelope.attack = i;
        if (param == "D") sid.voices[0].envelope.decay = i;
        if (param == "S") sid.voices[0].envelope.sustain = i;
        if (param == "R") sid.voices[0].envelope.release = i;

        std::cout << "Run test:"
                << " A:" << toHexStr(sid.voices[0].envelope.attack)[1]
                << " D:" << toHexStr(sid.voices[0].envelope.decay)[1]
                << " S:" << toHexStr(sid.voices[0].envelope.sustain)[1]
                << " R:" << toHexStr(sid.voices[0].envelope.release)[1] << std::endl;

        // 20s gate=1, 20s gate=0

        sid.voices[0].setGate(true);

        for(int j = 0; j < 20 * sampleRate; j++)
            nextSample();

        sid.voices[0].setGate(false);

        for(int j = 0; j < 20 * sampleRate; j++)
            nextSample();

    }

    std::cout << "tests completed." << std::endl;
    std::cout << "import in Audacity as binary data -> 32bit float stereo" << std::endl;

    return 0;
}

int sweepTest() {
    std::cout << "run SID test: sweep\n";

    const int sampleRate = 44'100;
    const int phiRate = 985'248;
    SID sid(sampleRate, phiRate);
    sid.volume = 0xF;
    sid.voices[1].test = true;
    sid.voices[2].test = true;
    sid.voices[0].test = false;
    sid.voices[0].setGate(true);
    sid.voices[0].envelope.attack = 0;
    sid.voices[0].envelope.decay = 0;
    sid.voices[0].envelope.sustain = 0xF;
    sid.voices[0].envelope.release = 0;

    std::string outpath = prompt("output path", "test.bin");
    std::cout << "waveforms: saw, triangle, pulse, noise\n";
    std::string waveform = prompt("waveform", "noise");

    if (waveform == "saw") sid.voices[0].saw = true;
    if (waveform == "triangle") sid.voices[0].triangle = true;
    if (waveform == "pulse") sid.voices[0].pulse = true;
    if (waveform == "noise") sid.voices[0].noise = true;

    std::fstream file(outpath, std::ios::out | std::ios::binary);

    double start_freq = static_cast<double>(0x0100);
    double end_freq = static_cast<double>(0xFFFF);
    for(int j = 0; j < 20 * sampleRate; j++) {
        if ((j % sampleRate) == 0) std::cout << j / sampleRate / 2 * 10 << "%\n";
        sid.voices[0].freq = (start_freq * pow(end_freq / start_freq,  static_cast<double>(j) / (20.0 * sampleRate - 1)));
        float sample;
        sid.process(1, &sample);
        file.write(reinterpret_cast<char*>(&sample), sizeof(float));
    }

    std::cout << "test completed." << std::endl;
    std::cout << "import in Audacity as binary data -> 32bit float mono" << std::endl;

    return 0;
}

int testsid() {
    std::cout << "SID test: supported tests: envelope sweep\n";
    std::cout << "test: [sweep] >";
    std::string mode = "";
    std::getline(std::cin, mode);
    if (mode == "") mode = "sweep";

    if (mode == "envelope")
        return envelopeTest();
    if (mode == "sweep")
        return sweepTest();

    std:: cout << "invalid mode entered." << std::endl;
    return 1;
}
