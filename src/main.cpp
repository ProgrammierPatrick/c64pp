#include "gui/main_window.h"
#include "gui/text_utils.h"

#include "emu/mpu.h"
#include "emu/debug/mpu_trace.h"

#include <QApplication>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <deque>
#include <chrono>


int functional_test(const char *filepath, const char *successAddrStr);
int help(const char *appName);

int main(int argc, char** argv) {
    if (argc > 1) {
        if (argv[1] == std::string("help")) return help(argv[0]);
        else if (argv[1] == std::string("functional_test")) {
            if (argc != 4) {
                std::cout << "Syntax error! expected: " << argv[0] << " functional_test [filename] [success_addr]" << std::endl;
                return -1;
            }
            return functional_test(argv[2], argv[3]);
        } else {
            std::cout << "Unknown command " << argv[1] << "! Call " << argv[0] << " help for help" << std::endl;
            return -1;
        }
    } else {
        QApplication app(argc, argv);
        MainWindow win;
        win.show();
        return app.exec();
    }
}

int help(const char *appName) {
    std::cout << appName << "                            open GUI application\n";
    std::cout << appName << " help                       show this help\n";
    std::cout << appName << " functional_test [filename] run functional test from https://github.com/Klaus2m5/6502_65C02_functional_tests (binary gets loaded as ram and PC starts at 0x400\n";
    std::cout << std::endl;
    return 0;
}

int functional_test(const char *filepath, const char *successAddrStr) {
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

    uint16_t successAddr = fromHexStr16(successAddrStr);

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
