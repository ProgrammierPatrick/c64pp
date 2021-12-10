#pragma once

#include "memory.h"

#include <vector>

class RAMMemory : public Memory {
public:
    RAMMemory(int size) : data(size) { }

    uint8_t read(uint16_t addr) override {
        if (addr < data.size())
            return data[addr];
        else
            return 0x00;
    }

    void write(uint16_t addr, uint8_t data) override {
        if (addr < this->data.size())
            this->data[addr] = data;
    }

    std::vector<uint8_t> data;
};
