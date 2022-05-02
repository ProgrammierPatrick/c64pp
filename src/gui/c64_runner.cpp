#include "c64_runner.h"

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
std::string loadResAsStr(const char* name) {
    QFile file(name);
    if(!file.open(QIODevice::ReadOnly)) {
        std::cout << "could not open resource " << name << std::endl;
        exit(1);
    }
    auto data = file.readAll();
    return std::string(data.begin(), data.end());
}

void C64Runner::hardReset() {
    auto basic = loadRes(":/roms/basic");
    auto kernal = loadRes(":/roms/kernal");
    auto chargen = loadRes(":/roms/chargen");

    auto keymap = loadResAsStr(":/keymaps/gtk3_sym_de.vkm");
    keyboard = std::make_unique<KeyboardState>(keymap);

    c64 = std::make_unique<C64>(basic, kernal, chargen, keyboard.get());
}

void C64Runner::singleStepMPU() {
    c64->tick();
}
int C64Runner::stepInstruction() {
    int numTicks = 0;
    do {
        c64->tick();
        numTicks++;
    } while (c64->mpu.T != 0);
    return numTicks;
}
int C64Runner::stepLine() {
    for (int i = 0; i < 63; i++)
        c64->tick();
    return 63;
}
int C64Runner::stepFrame() {
    // should be 19656: https://www.c64-wiki.com/wiki/raster_time
    for (int i = 0; i < 19656; i++)
        c64->tick();
    return 19656;

    //int numTicks = 0;
    //do {
    //    c64->tick();
    //    numTicks++;
    //} while(c64->vic.y != 0 || c64->vic.cycleInLine != 1);
    //return numTicks;
}

