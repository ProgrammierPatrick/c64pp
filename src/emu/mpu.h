#pragma once

#include "mem/memory.h"

class MPU {
public:
    MPU(Memory* memory) : mem(memory) { }

    void tick(bool IRQ = false, bool NMI = false);
    void reset() {
        // TODO: set mask interrupt flag
        auto PCL = mem->read(0xFFFC);
        auto PCH = mem->read(0xFFFD);
        PC = (static_cast<uint16_t>(PCH) << 8) | PCL;
    }
private:
    void get_lower_abs_address();
    void get_higher_abs_address();

public:
    // registers
    uint8_t A = 0;   // accumulator
    uint8_t X = 0;   // index
    uint8_t Y = 0;   // index
    uint8_t P = 0;   // status
    uint8_t S = 0;   // stack pointer
    uint16_t PC = 0; // program counter

    // status flags
    enum Flag : uint8_t {
        C = 0x01, // carry
        Z = 0x02, // zero
        I = 0x04, // IRQ disable
        D = 0x08, // decimal mode
        B = 0x10, // BRK command
        EmptyBit = 0x20,
        V = 0x40, // overflow
        N = 0x80, // negative
    };

    uint8_t opcode = 0;
    int T = 0;

    // temp values for addresses during opcode processing
    uint16_t effectiveAddr = 0;
    uint16_t baseAddr = 0;
    uint16_t indirectAddr = 0;

    uint8_t modVal = 0; // temp modify value used for read-modify-write operations
    int8_t offset = 0; // offset used for branching, signed 8 bit

    // MNI is edge sensitive. If NMI is already handled, it needs to return to zero before being detected again
    // NMI_valid will be false after NMI has been received, returns to true when NMI=0
    bool NMI_valid = true;

    // true when NMI is handled currently
    bool handlingIRQorNMI = false;
    bool handlingNMI = false;

    Memory* mem = nullptr;

private:


};
