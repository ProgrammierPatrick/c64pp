#include "gui/main_window.h"
#include "emu/mpu.h"
#include "gui/text_utils.h"

#include <QApplication>

#include <iostream>
#include <fstream>
#include <vector>

void functional_test(const char *filepath);
void help(const char *appName);

int main(int argc, char** argv) {
    if (argc > 1) {
        if (argv[1] == std::string("help")) help(argv[0]);
        else if (argv[1] == std::string("functional_test")) {
            if (argc != 3) {
                std::cout << "Syntax error! expected: " << argv[0] << " functional_test [filename]" << std::endl;
                return -1;
            }
            functional_test(argv[2]);
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

void help(const char *appName) {
    std::cout << appName << "                            open GUI application\n";
    std::cout << appName << " help                       show this help\n";
    std::cout << appName << " functional_test [filename] run functional test from https://github.com/Klaus2m5/6502_65C02_functional_tests (biary gets loaded as ram and PC starts at 0x400\n";
    std::cout << std::endl;
}

void functional_test(const char *filepath) {
    RAMMemory mem(64 * 1024 * 1024);
    MPU mpu(&mem);

    std::ifstream file(filepath, std::ios::binary);
    std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), mem.data.begin());
    file.close();

    mpu.PC = 0x400;
    auto lastPC = mpu.PC;
    auto lastCycle = mpu.cycle;
    for(int i = 0;;i++) {
        if (i % 100000 == 0) std::cout << "PC: " << toHexStr(mpu.PC) << " cycle: " << i << std::endl;

        mpu.tick();

        if (mpu.cycle == lastCycle) {
            std::cout << "MPU got stuck in cycle " << mpu.cycle << " at instruction " << toHexStr(mpu.PC) << ": ";
            std::cout << toHexStr(mem.read(mpu.PC)) << " " << toHexStr(mem.read(mpu.PC + 1)) << " " << toHexStr(mem.read(mpu.PC + 2)) << " " << toHexStr(mem.read(mpu.PC + 3)) << std::endl;
            break;
        }
        lastCycle = mpu.cycle;

        if (mpu.cycle == 0) {
            if (mpu.PC == lastPC) {
                std::cout << "trap encountered at PC:" << toHexStr(mpu.PC) << " in cycle " << i <<std::endl;
                break;
            } else {
                lastPC = mpu.PC;
            }
        }
    }
}
