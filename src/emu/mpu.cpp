#include "mpu.h"
#include <iostream>
#include <array>
/**
 * (Moritz)
 *
 * To Dos:
 *  - BCD Variants
 */

void dataHandlerNop(MPU& mpu, uint8_t data) { }

struct OpCode {
    // int numBytes; // length of istruction including opcode
    std::array<void (*)(MPU&), 7> handlers;
    void (*dataHandler)(MPU& mpu, uint8_t data) = dataHandlerNop;
};
extern std::array<OpCode, 256> opcodes;

void undefinedOpcode(MPU& mpu) {
    std::cout << "Undefined OpCode encountered: " << std::hex << (int)mpu.opcode << " at memory location " << (int)mpu.PC << std::endl;
    mpu.cycle = 0;
}

void fetchOpCode(MPU& mpu) {
    mpu.opcode = mpu.mem->read(mpu.PC);
    mpu.cycle++;
}

void handlerNop(MPU& mpu) {
    mpu.cycle++;
}

void fetchImmediate(MPU& mpu) {
    uint8_t value = mpu.mem->read(mpu.PC + 1);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.cycle = 0;
    mpu.PC += 2;
}
OpCode immediateMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    // opcodeData.numBytes = 2;
    opcodeData.handlers = { fetchOpCode, fetchImmediate, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchAbsoluteLowAddr(MPU& mpu) {
    mpu.effectiveAddr = mpu.mem->read(mpu.PC + 1);
    mpu.cycle++;
}
void fetchAbsoluteHighAddr(MPU& mpu) {
    mpu.effectiveAddr |= static_cast<uint16_t>(mpu.mem->read(mpu.PC + 2)) << 8;
    mpu.cycle++;
}
void fetchAbsoluteData(MPU& mpu) {
    uint8_t value = mpu.mem->read(mpu.effectiveAddr);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.cycle = 0;
    mpu.PC += 3;
}
OpCode absoluteMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    // opcodeData.numBytes = 3;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr, fetchAbsoluteData, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchZeroPageAddr(MPU& mpu) {
    mpu.effectiveAddr = mpu.mem->read(mpu.PC + 1);
    mpu.cycle++;
}
void fetchZeroPageData(MPU& mpu) {
    uint8_t value = mpu.mem->read(mpu.effectiveAddr);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.cycle = 0;
    mpu.PC += 2;
}
OpCode zeroPageMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageAddr, fetchZeroPageData, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchIndirectXBase(MPU& mpu) {
    mpu.baseAddr = mpu.mem->read(mpu.PC + 1);
}
void fetchIndirectXAddrLow(MPU& mpu) {
    mpu.effectiveAddr = mpu.mem->read(mpu.baseAddr + mpu.X);
}
void fetchIndirectXAddrHigh(MPU& mpu) {
    mpu.effectiveAddr |= mpu.mem->read(mpu.baseAddr + mpu.X + 1) << 8;
}
void fetchIndirectXData(MPU& mpu) {
    uint8_t value = mpu.mem->read(mpu.effectiveAddr);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.cycle = 0;
    mpu.PC += 2;
}
OpCode indirectXMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchIndirectXBase, handlerNop, fetchIndirectXAddrLow, fetchIndirectXAddrHigh, fetchIndirectXData, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void opADC(MPU& mpu, uint8_t value) {
    uint8_t oldA = mpu.A;
    mpu.A = oldA + value + (mpu.P & MPU::Flag::C);
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    bool carry = (oldA + static_cast<uint16_t>(value) + (mpu.P & MPU::Flag::C)) > 0x00FF;
    bool overflow = (oldA + static_cast<int16_t>(value) + (mpu.P & MPU::Flag::C)) != mpu.A;
    mpu.P &= ~(MPU::Flag::C | MPU::Flag::Z | MPU::Flag::V | MPU::Flag::N);
    mpu.P |= (carry ? MPU::Flag::C : 0) | (zero ? MPU::Flag::Z : 0) | (overflow ? MPU::Flag::V : 0) | (neg ? MPU::Flag::N : 0);
}

void opAND(MPU& mpu, uint8_t value) {
    mpu.A &= value;
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
}

void opBIT(MPU& mpu, uint8_t value) {
    bool neg = 0x80 & value;
    bool overflow = 0x40 & value;
    bool zero = (0x3F & mpu.A & value) == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::V | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (overflow ? MPU::Flag::V : 0) |(zero ? MPU::Flag::Z : 0);
}

void opCMP(MPU& mpu, uint8_t value) {
    int8_t diff = mpu.A - value;
    bool neg = diff & 0x80;
    bool zero = diff == 0;
    bool carry = static_cast<int16_t>(mpu.A) - static_cast<int16_t>(value) < 0;
    mpu.P &= ~(MPU::Flag::C | MPU::Flag::Z | MPU::Flag::N);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (carry ? MPU::Flag::C : 0) |(zero ? MPU::Flag::Z : 0);
}

void opCPX(MPU& mpu, uint8_t value) {
    int8_t diff = mpu.X - value;
    bool neg = diff & 0x80;
    bool zero = diff == 0;
    bool carry = static_cast<int16_t>(mpu.X) - static_cast<int16_t>(value) < 0;
    mpu.P &= ~(MPU::Flag::C | MPU::Flag::Z | MPU::Flag::N);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (carry ? MPU::Flag::C : 0) |(zero ? MPU::Flag::Z : 0);
}

void opCPY(MPU& mpu, uint8_t value) {
    int8_t diff = mpu.Y - value;
    bool neg = diff & 0x80;
    bool zero = diff == 0;
    bool carry = static_cast<int16_t>(mpu.Y) - static_cast<int16_t>(value) < 0;
    mpu.P &= ~(MPU::Flag::C | MPU::Flag::Z | MPU::Flag::N);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (carry ? MPU::Flag::C : 0) |(zero ? MPU::Flag::Z : 0);
}

void opEOR(MPU& mpu, uint8_t value) {
    mpu.A ^= value;
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
}

void opLDA(MPU& mpu, uint8_t value) {
    mpu.A = value;
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
}

void opLDX(MPU& mpu, uint8_t value) {
    mpu.X = value;
    bool neg = static_cast<int8_t>(mpu.X) < 0;
    bool zero = mpu.X == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
}

void opLDY(MPU& mpu, uint8_t value) {
    mpu.Y = value;
    bool neg = static_cast<int8_t>(mpu.Y) < 0;
    bool zero = mpu.Y == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
}

void opORA(MPU& mpu, uint8_t value) {
    mpu.A |= value;
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
}

void opSBC(MPU& mpu, uint8_t value) {
    uint8_t oldA = mpu.A;
    mpu.A = mpu.A - value - ((mpu.P & MPU::Flag::C) ^ 0x01);
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    bool carry = (oldA - static_cast<uint16_t>(value) - (0x01 ^ (mpu.P & MPU::Flag::C))) > 0x00FF;
    bool overflow = (oldA - static_cast<int16_t>(value) - (0x01 ^ (mpu.P & MPU::Flag::C))) != mpu.A;
    mpu.P &= ~(MPU::Flag::C | MPU::Flag::Z | MPU::Flag::V | MPU::Flag::N);
    mpu.P |= (carry ? MPU::Flag::C : 0) | (zero ? MPU::Flag::Z : 0) | (overflow ? MPU::Flag::V : 0) | (neg ? MPU::Flag::N : 0);
}


// BRK instruction (called for hardware interrupt)
void brkPushPCH(MPU& mpu) {
    mpu.mem->write(0x0100 | mpu.S, mpu.PC & 0xFF);
    mpu.S--;
}
void brkPushPCL(MPU& mpu) {
    mpu.mem->write(0x0100 | mpu.S, (mpu.PC >> 8) & 0xFF);
    mpu.S--;
}
void brkPushP(MPU& mpu) {
    mpu.mem->write(0x0100 | mpu.S, mpu.P);
    mpu.S--;
}
void brkFetchAddrLow(MPU& mpu) {
    mpu.PC = mpu.mem->read(mpu.handlingNMI ? 0xFFFA : 0xFFFE);
}
void brkFetchAddrHigh(MPU& mpu) {
    mpu.PC |= mpu.mem->read(mpu.handlingNMI ? 0xFFFB : 0xFFFF) << 8;
    mpu.handlingNMI = false;
    mpu.cycle = 0;
}
OpCode createBRKOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, handlerNop, brkPushPCH, brkPushPCL, brkPushP, brkFetchAddrLow, brkFetchAddrHigh };
    return opcodeData;
}

// RTI instruction (return from interrupt)
void rtiPullP(MPU& mpu) {
    mpu.S--;
    mpu.P = mpu.mem->read(0x0100 | mpu.S);
}
void rtiPullPCL(MPU& mpu) {
    mpu.S--;
    mpu.PC = mpu.mem->read(0x0100 | mpu.S);
}
void rtiPullPCH(MPU& mpu) {
    mpu.S--;
    mpu.PC |= mpu.mem->read(0x0100 | mpu.S) << 8;
    mpu.cycle = 0;
}
OpCode createRTIOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, handlerNop, handlerNop, rtiPullP, rtiPullPCL, rtiPullPCH, undefinedOpcode };
    return opcodeData;
}

std::array<OpCode, 256> createOpcodes() {
    std::array<OpCode, 256> opcodes{};
    for(auto& op : opcodes) op.handlers = { fetchOpCode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };

    // Internal Execution On Memory Data
    opcodes[0x69] = immediateMode(opADC);
    opcodes[0x6D] = absoluteMode(opADC);
    opcodes[0x65] = zeroPageMode(opADC);
    opcodes[0x61] = indirectXMode(opADC);
    opcodes[0x29] = immediateMode(opAND);
    opcodes[0x2D] = absoluteMode(opAND);
    opcodes[0x25] = zeroPageMode(opAND);
    opcodes[0x21] = indirectXMode(opAND);
    opcodes[0x2C] = absoluteMode(opBIT);
    opcodes[0x24] = zeroPageMode(opBIT);
    opcodes[0xC9] = immediateMode(opCMP);
    opcodes[0xCD] = absoluteMode(opCMP);
    opcodes[0xC5] = zeroPageMode(opCMP);
    opcodes[0xC1] = indirectXMode(opCMP);
    opcodes[0xE0] = immediateMode(opCPX);
    opcodes[0xEC] = absoluteMode(opCPX);
    opcodes[0xE4] = zeroPageMode(opCPX);
    opcodes[0xC0] = immediateMode(opCPY);
    opcodes[0xCC] = absoluteMode(opCPY);
    opcodes[0xC4] = zeroPageMode(opCPY);
    opcodes[0x49] = immediateMode(opEOR);
    opcodes[0x4D] = absoluteMode(opEOR);
    opcodes[0x45] = zeroPageMode(opEOR);
    opcodes[0x41] = indirectXMode(opEOR);
    opcodes[0xA9] = immediateMode(opLDA);
    opcodes[0xAD] = absoluteMode(opLDA);
    opcodes[0xA5] = zeroPageMode(opLDA);
    opcodes[0xA1] = indirectXMode(opLDA);
    opcodes[0xA2] = immediateMode(opLDX);
    opcodes[0xAE] = absoluteMode(opLDX);
    opcodes[0xA6] = zeroPageMode(opLDX);
    opcodes[0xA0] = immediateMode(opLDY);
    opcodes[0xAC] = absoluteMode(opLDY);
    opcodes[0xA4] = zeroPageMode(opLDY);
    opcodes[0x09] = immediateMode(opORA);
    opcodes[0x0D] = absoluteMode(opORA);
    opcodes[0x05] = zeroPageMode(opORA);
    opcodes[0x01] = indirectXMode(opORA);
    opcodes[0xE9] = immediateMode(opSBC);
    opcodes[0xED] = absoluteMode(opSBC);
    opcodes[0xE5] = zeroPageMode(opSBC);
    opcodes[0xE1] = indirectXMode(opSBC);

    opcodes[0x00] = createBRKOpCode();
    opcodes[0x40] = createBRKOpCode();

    return opcodes;
}
std::array<OpCode, 256> opcodes = createOpcodes();

void MPU::tick(bool IRQ, bool NMI) {
    if (!NMI) NMI_valid = true;

    if (cycle == 0) {
        if (NMI_valid && NMI)
            handlingNMI = true;

        if (!(P & Flag::I) && IRQ || handlingNMI) {
            if(handlingNMI) NMI_valid = false;
            opcode = 0x00; // BRK
        }
    }

    opcodes[opcode].handlers[cycle](*this);
}
