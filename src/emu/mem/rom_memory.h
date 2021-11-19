#pragma once

#include "memory.h"

#include <vector>
#include <string>

class RomMemory : public Memory {
public:
    RomMemory(const std::vector<uint8_t>& data) : data(data) { }
    RomMemory(const std::string& filename);

    uint8_t read(uint16_t addr) override {
        if (addr < data.size())
            return data[addr];
        else
            return 0x00;
    }

    void write(uint16_t addr, uint8_t data) override { }
private:
    std::vector<uint8_t> data;
};
