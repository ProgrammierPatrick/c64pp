#include "testsid.h"

#include "../emu/sid/sid.h"

#include "../gui/text_utils.h"

#include <iostream>
#include <fstream>

int testsid() {
    std::cout << "run SID test: envelope\n";

    const int sampleRate = 44'100;
    const int phiRate = 985'248;
    SID sid(sampleRate, phiRate);
    sid.voices[1].test = true;
    sid.voices[2].test = true;
    sid.volume = 0xF;
    sid.voices[0].freq = 7382; // 440Hz at 1MHz, 7382*985248/16777216=443,511Hz
    sid.voices[0].saw = true;

    std::cout << "output path: [test.bin] > ";
    std::string outpath = "";
    std::getline(std::cin, outpath);
    if (outpath == "") outpath = "test.bin";

    std::cout << "sweep parameter (A,D,S,R) [A] > ";
    std::string param = "";
    std::getline(std::cin, param);
    if (param == "") param = "A";
    if (param[0] >= 'a' && param[0] <= 'z') param[0] -= ('a' - 'A');
    if (!(param == "A" || param == "D" || param == "S" || param == "R")) {
        std::cout << "invalid param selected. exiting.";
        return 1;
    }
    if (param != "A") {
        std::cout << "value for A: [A] > ";
        std::string in;
        std::getline(std::cin, in);
        if (in == "") in = "A";
        in = "0"+in;
        sid.voices[0].envelope.attack = fromHexStr8(in);
    }
    if (param != "D") {
        std::cout << "value for D: [A] > ";
        std::string in;
        std::getline(std::cin, in);
        if (in == "") in = "A";
        in = "0"+in;
        sid.voices[0].envelope.decay = fromHexStr8(in);
    }
    if (param != "S") {
        std::cout << "value for S: [7] > ";
        std::string in;
        std::getline(std::cin, in);
        if (in == "") in = "7";
        in = "0"+in;
        sid.voices[0].envelope.sustain = fromHexStr8(in);
    }
    if (param != "R") {
        std::cout << "value for R: [7] > ";
        std::string in;
        std::getline(std::cin, in);
        if (in == "") in = "7";
        in = "0"+in;
        sid.voices[0].envelope.release = fromHexStr8(in);
    }

    std::fstream file(outpath, std::ios::out | std::ios::binary);

    auto nextSample = [&file, &sid]() {
        float value;
        float sample;
        double sampleDouble;
        sid.process(1, &sampleDouble);
        sample = sampleDouble;
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
