#include "trace.h"

#include "../gui/c64_runner.h"
#include "../gui/text_utils.h"

#include <iostream>
#include <fstream>
#include <vector>

int trace(const std::string& filepath, uint16_t startAddr) {
    C64Runner c64Runner;
    auto& c64 = *c64Runner.c64;
    auto& mpu = c64Runner.c64->mpu;

    // wait long enough to initialize BASIC
    for (int i = 0; i < 150; i++)
        c64Runner.stepFrame();

    std::ifstream file(filepath, std::ios::binary);
    file.unsetf(std::ios::skipws);

    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize == -1) {
        std::cout << "Error: file " << filepath << " could not be found." << std::endl;
    }
    std::vector<uint8_t> data{std::istream_iterator<char>{file}, std::istream_iterator<char>{}};
    file.close();

    uint16_t fileStartAddr = data[0] | (data[1] << 8);

    for (int i = 2; i < data.size(); i++) {
        mpu.mem->write(fileStartAddr + i - 2, data[i]);
    }
    std::cout << data.size() - 2 << "bytes loaded to memory." << std::endl;

    mpu.PC = startAddr;
    mpu.T = 0;
    std::cout << "set PC to start addr " << toHexStr(mpu.PC) << std::endl;

    for(int i = 0; i < 100000000; i++) {
        c64Runner.stepInstruction();
        std::cout << ".C:" << toHexStr(mpu.PC) << "  "
                  << toHexStr(mpu.mem->read(mpu.PC, true)) << " "
                  << toHexStr(mpu.mem->read(mpu.PC + 1, true)) << " "
                  << toHexStr(mpu.mem->read(mpu.PC + 2, true)) << "    "
                  << "               - A:" << toHexStr(mpu.A) << " X:" << toHexStr(mpu.X) << " Y:" << toHexStr(mpu.Y)
                  << " SP:" << toHexStr(mpu.S) << " " << ((mpu.P & MPU::N) ? 'N' : '.') << ((mpu.P & MPU::V) ? 'V' : '.')
                  << "-" << ((mpu.P & MPU::B) ? 'B' : '.') << ((mpu.P & MPU::D) ? 'D' : '.') << ((mpu.P & MPU::I) ? 'I' : '.')
                  << ((mpu.P & MPU::Z) ? 'Z' : '.') << ((mpu.P & MPU::C) ? 'C' : '.') << std::endl;
    }
    return 0;
}
