#include "functional_test.h"

#include "../emu/mem/ram_memory.h"
#include "../emu/mpu.h"
#include "../emu/debug/mpu_trace.h"

#include "../gui/text_utils.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <iterator>

int functional_test(const std::string& filepath, uint16_t successAddr) {
    RAMMemory mem(64 * 1024 * 1024);
    MPU mpu(&mem);
    MPUTrace trace(&mpu);

    std::ifstream file(filepath, std::ios::binary);
    file.unsetf(std::ios::skipws);

    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize == -1) {
        std::cout << "Error: file " << filepath << " could not be found." << std::endl;
        return -1;
    }

    // program starts at 0x000A
    std::copy(std::istream_iterator<char>(file), std::istream_iterator<char>(), mem.data.begin() + 0x000A);
    std::cout << fileSize << " bytes loaded." << std::endl;

    mpu.PC = 0x400;
    int instr = 0;
    int i;

    auto startTime = std::chrono::high_resolution_clock::now();
    auto printTime = [&]() {
        auto diff = std::chrono::high_resolution_clock::now() - startTime;
        auto t = std::chrono::duration_cast<std::chrono::seconds>(diff);
        float freq = i / static_cast<float>(t.count()) / 1000000;
        std::cout << "Runtime: " << t.count() << "s, " << freq << "MHz average" << std::endl;
    };

    for(i = 0;;i++) {

        trace.tick();

        if ((i % 10000 == 0) && (i < 1000000) || (i % 100000 == 0)) std::cout << "current cycle: " << i << std::endl;

        mpu.tick();

        if (mpu.T == trace.lastT) {
            trace.printHistoryToConsole();
            std::cout << "MPU got stuck in mpu sub-cycle " << mpu.T << " at PC " << toHexStr(mpu.PC) << ": ";
            std::cout << toHexStr(mem.read(mpu.PC)) << " " << toHexStr(mem.read(mpu.PC + 1)) << " " << toHexStr(mem.read(mpu.PC + 2)) << std::endl;

            return 1;
        }

        if (mpu.PC == successAddr) {
            trace.printHistoryToConsole();
            std::cout << "Test finished successfully, PC " << toHexStr(successAddr) << "has been reached." << std::endl;
            trace.printHistoryToConsole();
            printTime();
            return 0;
        }

        if (mpu.T == 0) {
            if (trace.lastPC == trace.lastLastPC) {
                trace.printHistoryToConsole();
                std::cout << "trap encountered at PC:" << toHexStr(mpu.PC) << " in cycle " << i <<std::endl;
                std::cout << toHexStr(mpu.PC) << ": " << toHexStr(mem.read(mpu.PC)) << " " << toHexStr(mem.read(mpu.PC + 1)) << " " << toHexStr(mem.read(mpu.PC + 2)) << std::endl;
                printTime();

                while(1) {
                    std::cout << "Enter ram addr to examine (or END to end) > " << std::flush;
                    std::string s;
                    std::cin >> s;
                    if (s == "END") return 1;
                    auto addr = fromHexStr16(s) & 0xFFF0;
                    for(int y = 0; y < 16; y++) {
                        std::cout << toHexStr(static_cast<uint16_t>(addr + y * 16)) << ": ";
                        for (int x = 0; x < 16; x++) {
                            std::cout << toHexStr(mem.read(addr + y * 16 + x)) << " ";
                        }
                        std::cout << "\n";
                    }
                    std::cout << std::endl;
                }

                return 1;
            }
            instr++;
        }
    }

    return 0; // unreachable


}
