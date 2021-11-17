#pragma once

#include "mem/memory.h"

class MPU {
public:
    MPU(Memory* memory) : mem(memory) { }

    void tick();
    void reset() {
        // TODO: set mask interrupt flag
        PCL = mem->read(0xFFFC);
        PCH = mem->read(0xFFFD);
    }
private:
    uint16_t pc() { return PCH * (1<<8) | PCL; }
    void pc_inc() {
        if (PCL == 0xFF) {
            PCL = 0;
            PCH++;
        } else {
            PCL++;
        }
    }

public:
    // registers
    uint8_t A; // accumulator
    uint8_t X; // index
    uint8_t Y; // index
    uint8_t P; // status
    uint8_t S; // stack pointer
    uint8_t PCH; // program counter high
    uint8_t PCL; // program counter low

private:
    // status flags
    enum Flag : uint8_t {
        C = 0x01, // carry
        Z = 0x02, // zero
        I = 0x04, // IRQ disable
        D = 0x08, // decimal mode
        B = 0x10, // BRK command
        V = 0x40, // overflow
        N = 0x80, // negative
    };

    uint8_t instr;
    int cycle = 0;

    Memory* mem;
};
