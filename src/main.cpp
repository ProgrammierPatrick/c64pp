#include "gui/main_window.h"
#include "emu/mpu.h"
#include "gui/text_utils.h"

#include <QApplication>

#include <iostream>
#include <fstream>
#include <vector>
#include<algorithm>

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
    auto lastlastPC = mpu.PC;
    auto lastPC = mpu.PC;
    auto lastCycle = mpu.cycle;
    int instr = 0;
    for(int i = 0;;i++) {

        if (mpu.cycle == 0) {
            std::cout << "INSTR " << toHexStr(mpu.PC) << ": " << toHexStr(mem.read(mpu.PC)) << " " << toHexStr(mem.read(mpu.PC + 1)) << " " << toHexStr(mem.read(mpu.PC + 2));
            std::cout << " instruction: " << instr << " cycle: " << i << '\n';
        }

        std::cout << "PC:" << toHexStr(mpu.PC) << " A:" << toHexStr(mpu.A) << " X:" << toHexStr(mpu.X) << " Y:"<< toHexStr(mpu.Y);
        std::cout << "  cycle:" << i << '\n';

        mpu.tick();

        if (mpu.cycle == lastCycle) {
            std::cout << "MPU got stuck in mpu sub-cycle " << mpu.cycle << " at PC " << toHexStr(mpu.PC) << ": ";
            std::cout << toHexStr(mem.read(mpu.PC)) << " " << toHexStr(mem.read(mpu.PC + 1)) << " " << toHexStr(mem.read(mpu.PC + 2)) << std::endl;
            break;
        }
        lastCycle = mpu.cycle;

        if (mpu.PC == successAddr) {
            std::cout << "Test finished successfully, PC " << toHexStr(successAddr) << "has been reached.";
            return 0;
        }

        if (mpu.cycle == 0) {
            if (mpu.PC == lastPC) {
                std::cout << "trap encountered at PC:" << toHexStr(mpu.PC) << " in cycle " << i <<std::endl;
                std::cout << toHexStr(mpu.PC) << ": " << toHexStr(mem.read(mpu.PC)) << " " << toHexStr(mem.read(mpu.PC + 1)) << " " << toHexStr(mem.read(mpu.PC + 2)) << std::endl;
                std::cout << "Instruction before trap: " << toHexStr(lastlastPC) << ": " << toHexStr(mem.read(lastlastPC)) << " " << toHexStr(mem.read(lastlastPC + 1)) << " " << toHexStr(mem.read(lastlastPC + 2)) << std::endl;
                return 1;
            } else {
                lastlastPC = lastPC;
                lastPC = mpu.PC;
            }
            instr++;
        }
    }
    return 0; // unreachable
}
