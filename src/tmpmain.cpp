#include "emu/mpu.h"
#include "emu/mem/ram_memory.h"
#include "emu/c64.h"

#include <QFile>

#include <iostream>


std::vector<uint8_t> loadRes(const char* name) {
    QFile file(name);
    if(!file.open(QIODevice::ReadOnly)) {
        std::cout << "could not open resource " << name << std::endl;
        exit(1);
    }
    auto data = file.readAll();
    return std::vector<uint8_t>(data.begin(), data.end());

}

int main() {
    auto basic = loadRes(":/roms/basic");
    auto kernal = loadRes(":/roms/kernal");
    auto chargen = loadRes(":/roms/chargen");

    C64 c64(basic, kernal, chargen);

    c64.mainRAM.write(2, 0x69);
    c64.mainRAM.write(3, 3);
    c64.mainRAM.write(4, 0x6D);
    c64.mainRAM.write(5, 0x07);
    c64.mainRAM.write(6, 0x00);
    c64.mainRAM.write(7, 12);

    MPU& mpu = c64.mpu;
    mpu.PCL = 2;
    mpu.PCH = 0;
    mpu.A = 0;
    mpu.P = 1;
    mpu.tick();
    mpu.tick();
    std::cout << "A: " << (int)mpu.A << std::endl;
    mpu.P = 1;
    mpu.tick();
    mpu.tick();
    mpu.tick();
    std::cout << "A: " << (int)mpu.A << std::endl;
}
