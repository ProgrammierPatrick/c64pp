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

class NullKeyboard : public Keyboard {
public:
    uint8_t query(uint8_t mask) override { return 0x00; }
};

void C64Runner::hardReset() {
    auto basic = loadRes(":/roms/basic");
    auto kernal = loadRes(":/roms/kernal");
    auto chargen = loadRes(":/roms/chargen");

    static NullKeyboard keyboard;
    c64 = std::make_unique<C64>(basic, kernal, chargen, &keyboard);
}
