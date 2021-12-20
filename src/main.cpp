#include "gui/main_window.h"
#include "emu/mpu.h"
#include "gui/text_utils.h"

#include <QApplication>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <deque>


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

    std::deque<std::string> outputLines;
    std::stringstream ss;
    auto printOutput = [&]() {
        outputLines.push_back(ss.str());
        for(auto& str : outputLines)
            std::cout << str;
        std::cout << std::endl;
    };

    uint16_t successAddr = fromHexStr16(successAddrStr);

    mpu.PC = 0x400;
    auto lastlastPC = mpu.PC;
    auto lastPC = mpu.PC;
    auto lastCycle = mpu.cycle;
    int instr = 0;
    for(int i = 0;;i++) {

        if (mpu.cycle == 0) {
            // DEC, INC, STA, STX, STY

            // INC, DEC Absolute
            if (mpu.opcode == 0xEE || mpu.opcode == 0xCE) {
                uint16_t addr = mem.read(lastlastPC + 1) | (mem.read(lastlastPC + 2) << 8);
                ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mem.read(addr)) << "\n";
            }
            // INC, DEC zeropage
            if (mpu.opcode == 0xE6 || mpu.opcode == 0xC6) {
                uint8_t addr = mem.read(lastlastPC + 1);
                ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mem.read(addr)) << "\n";
            }
            // INC, DEC zeropage, x
            if (mpu.opcode == 0xF6 || mpu.opcode == 0xD6) {
                ss << "MEMORY [????] = ?? (INC / DEC zeropage, x)\n";
            }
            // INC, DEC absolute x
            if (mpu.opcode == 0xFE || mpu.opcode == 0xDE) {
                ss << "MEMORY [????] = ?? (INC / DEC absolute x)\n";
            }
            // STA, STX, STY Absolute
            if (mpu.opcode == 0x8C || mpu.opcode == 0x8D || mpu.opcode == 0x8E) {
                uint16_t addr = mem.read(lastlastPC + 1) | (mem.read(lastlastPC + 2) << 8);
                ss << "lastLastPC: " << toHexStr(lastlastPC) << '\n';
                ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mem.read(addr)) << "\n";
            }
            // STA, STX, STY zeropage
            if (mpu.opcode == 0x84 || mpu.opcode == 0x85 || mpu.opcode == 0x86) {
                uint8_t addr = mem.read(lastlastPC + 1);
                ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mem.read(addr)) << "\n";
            }
            // STA (indexed) absolute X
            if (mpu.opcode == 0x9D) {
                uint16_t addr = (mem.read(lastlastPC + 1) | (mem.read(lastlastPC + 2) << 8)) + mpu.X;
                ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mem.read(addr)) << "\n";
            }
            // STA (indexed) absolute Y
            if (mpu.opcode == 0x99) {
                uint16_t addr = (mem.read(lastlastPC + 1) | (mem.read(lastlastPC + 2) << 8)) + mpu.Y;
                ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mem.read(addr)) << "\n";
            }
            // STA indirect Y
            if (mpu.opcode == 0x91) {
                auto indirectAddr = mpu.mem->read(lastlastPC + 1);
                uint16_t addr = (mpu.mem->read(indirectAddr) | (mpu.mem->read(indirectAddr + 1) << 8)) + mpu.Y;
                ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mem.read(addr)) << "    [" << indirectAddr << "] = " << toHexStr(mem.read(indirectAddr)) << "\n";
            }
            // STA indirect X
            if (mpu.opcode == 0x81) {
                // uint16_t addr = (mpu.mem->read(lastlastPC + 1) | (mpu.mem->read(lastlastPC + 2) << 8)) + mpu.X;
                uint8_t baseAddr = mem.read(lastlastPC + 1);
                uint16_t addr = mem.read((baseAddr + mpu.X) & 0x00FF) | (mem.read((baseAddr + mpu.X + 1) & 0x00FF) << 8);
                ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mem.read(addr)) << "    [" << baseAddr << "] = " << mem.read(baseAddr) << "\n";
            }
            // STA zeropage X, STY zeropage X
            if (mpu.opcode == 0x95 || mpu.opcode == 0x94) {
                uint16_t addr = (mpu.mem->read(lastlastPC + 1) + mpu.X) & 0x00FF;
                ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mem.read(addr)) << "\n";
            }
            // STX zeropage Y
            if (mpu.opcode == 0x96) {
                uint16_t addr = (mpu.mem->read(lastlastPC + 1) + mpu.Y) & 0x00FF;
                ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mem.read(addr)) << "\n";
            }

            ss << "Stack: " << "S:" << toHexStr(mpu.S) << "    " << toHexStr(mem.read(0x100 + mpu.S - 3)) << " " << toHexStr(mem.read(0x100 + mpu.S - 2)) << " " << toHexStr(mem.read(0x100 + mpu.S - 1));
            ss << " [" << toHexStr(mem.read(0x100 + mpu.S)) << "] " << toHexStr(mem.read(0x100 + mpu.S + 1)) << " " << toHexStr(mem.read(0x100 + mpu.S + 2)) << " " << toHexStr(mem.read(0x100 + mpu.S + 3)) << '\n';

            ss << "INSTR " << toHexStr(mpu.PC) << ": " << toHexStr(mem.read(mpu.PC)) << " " << toHexStr(mem.read(mpu.PC + 1)) << " " << toHexStr(mem.read(mpu.PC + 2));
            ss << " instruction: " << instr << " cycle: " << i << '\n';

        }

        if (i % 10000 == 0) std::cout << "current cycle: " << i << std::endl;

        ss << "  A:" << toHexStr(mpu.A);
        ss << " X:" << toHexStr(mpu.X);
        ss << " Y:" << toHexStr(mpu.Y);
        ss << " S:" << toHexStr(mpu.S);
        ss << " P:" << (mpu.P & 0x80 ? 'N' : '-') << (mpu.P & 0x40 ? 'V' : '-');
        ss << '-' << (mpu.P & 0x10 ? 'B' : '-') << (mpu.P & 0x08 ? 'D' : '-');
        ss << (mpu.P & 0x04 ? 'I' : '-') << (mpu.P & 0x02 ? 'Z' : '-') << (mpu.P & 0x01 ? 'C' : '-');
        ss << " PC:" << toHexStr(mpu.PC) << '\n';

        if(outputLines.size() >= 2000)
            outputLines.pop_front();
        outputLines.push_back(ss.str());
        ss.str("");

        mpu.tick();

        if (mpu.cycle == lastCycle) {
            ss << "MPU got stuck in mpu sub-cycle " << mpu.cycle << " at PC " << toHexStr(mpu.PC) << ": ";
            ss << toHexStr(mem.read(mpu.PC)) << " " << toHexStr(mem.read(mpu.PC + 1)) << " " << toHexStr(mem.read(mpu.PC + 2)) << std::endl;
            printOutput();
            return 1;
        }
        lastCycle = mpu.cycle;

        if (mpu.PC == successAddr) {
            ss << "Test finished successfully, PC " << toHexStr(successAddr) << "has been reached.";
            printOutput();
            return 0;
        }

        if (mpu.cycle == 0) {
            if (mpu.PC == lastPC) {
                ss << "trap encountered at PC:" << toHexStr(mpu.PC) << " in cycle " << i <<std::endl;
                ss << toHexStr(mpu.PC) << ": " << toHexStr(mem.read(mpu.PC)) << " " << toHexStr(mem.read(mpu.PC + 1)) << " " << toHexStr(mem.read(mpu.PC + 2)) << std::endl;
                ss << "Instruction before trap: " << toHexStr(lastlastPC) << ": " << toHexStr(mem.read(lastlastPC)) << " " << toHexStr(mem.read(lastlastPC + 1)) << " " << toHexStr(mem.read(lastlastPC + 2)) << std::endl;
                printOutput();

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
            } else {
                lastlastPC = lastPC;
                lastPC = mpu.PC;
            }
            instr++;
        }
    }

    return 0; // unreachable
}
