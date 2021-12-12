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
    mpu.effectiveAddr |= mpu.mem->read(mpu.PC + 2) << 8;
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
    mpu.cycle++;
}
void fetchIndirectXAddrLow(MPU& mpu) {
    mpu.effectiveAddr = mpu.mem->read((mpu.baseAddr + mpu.X) & 0x00FF);
    mpu.cycle++;
}
void fetchIndirectXAddrHigh(MPU& mpu) {
    mpu.effectiveAddr |= mpu.mem->read((mpu.baseAddr + mpu.X + 1) & 0x00FF) << 8;
    mpu.cycle++;
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

void fetchAbsoluteXAddrLow(MPU& mpu) {
    mpu.baseAddr = mpu.mem->read(mpu.PC + 1);
    mpu.cycle++;
}
void fetchAbsoluteXAddrHigh(MPU& mpu) {
    mpu.baseAddr |= mpu.mem->read(mpu.PC + 2) << 8;
    mpu.cycle++;
}
void fetchAbsoluteXData(MPU& mpu) {
    uint8_t value = mpu.mem->read(mpu.baseAddr + mpu.X);
    bool boundaryCrossed = (mpu.baseAddr & 0x00FF) + mpu.X > 0x00FF;
    if (boundaryCrossed) {
        mpu.cycle++;
    } else {
        opcodes[mpu.opcode].dataHandler(mpu, value);
        mpu.cycle = 0;
        mpu.PC += 3;
    }
}
void fetchAbsoluteXDataNextPage(MPU& mpu) {
    uint8_t value = mpu.mem->read(mpu.baseAddr + mpu.X);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.cycle = 0;
    mpu.PC += 3;
}
OpCode absoluteXMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteXAddrLow, fetchAbsoluteXAddrHigh, fetchAbsoluteXData, fetchAbsoluteXDataNextPage, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchAbsoluteYAddrLow(MPU& mpu) {
    mpu.baseAddr = mpu.mem->read(mpu.PC + 1);
    mpu.cycle++;
}
void fetchAbsoluteYAddrHigh(MPU& mpu) {
    mpu.baseAddr |= mpu.mem->read(mpu.PC + 2) << 8;
    mpu.cycle++;
}
void fetchAbsoluteYData(MPU& mpu) {
    uint8_t value = mpu.mem->read(mpu.baseAddr + mpu.Y);
    bool boundaryCrossed = (mpu.baseAddr & 0x00FF) + mpu.Y > 0x00FF;
    if (boundaryCrossed) {
        mpu.cycle++;
    } else {
        opcodes[mpu.opcode].dataHandler(mpu, value);
        mpu.cycle = 0;
        mpu.PC += 3;
    }
}
void fetchAbsoluteYDataNextPage(MPU& mpu) {
    uint8_t value = mpu.mem->read(mpu.baseAddr + mpu.Y);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.cycle = 0;
    mpu.PC += 3;
}
OpCode absoluteYMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteYAddrLow, fetchAbsoluteYAddrHigh, fetchAbsoluteYData, fetchAbsoluteYDataNextPage, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchZeroPageXBase(MPU& mpu) {
    mpu.baseAddr = mpu.mem->read(mpu.PC + 1);
    mpu.cycle++;
}
void fetchZeroPageXData(MPU& mpu) {
    uint8_t value = mpu.mem->read((mpu.baseAddr + mpu.X) & 0x00FF);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.cycle = 0;
    mpu.PC += 2;
}
OpCode zeroPageXMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageXBase, handlerNop, fetchZeroPageXData, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchZeroPageYBase(MPU& mpu) {
    mpu.baseAddr = mpu.mem->read(mpu.PC + 1);
    mpu.cycle++;
}
void fetchZeroPageYData(MPU& mpu) {
    uint8_t value = mpu.mem->read((mpu.baseAddr + mpu.Y) & 0x00FF);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.cycle = 0;
    mpu.PC += 2;
}
OpCode zeroPageYMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageYBase, handlerNop, fetchZeroPageYData, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchIndirectYIndirectAddr(MPU& mpu) {
    mpu.indirectAddr = mpu.mem->read(mpu.PC + 1);
    mpu.cycle++;
}
void fetchIndirectYAddrLow(MPU& mpu) {
    mpu.baseAddr = mpu.mem->read(mpu.indirectAddr);
    mpu.cycle++;
}
void fetchIndirectYAddrHigh(MPU& mpu) {
    mpu.baseAddr |= mpu.mem->read((mpu.indirectAddr + 1) & 0x00FF) << 8;
    mpu.cycle++;
}
void fetchIndirectYData(MPU& mpu) {
    uint8_t value = mpu.mem->read(mpu.baseAddr + mpu.Y);
    bool boundaryCrossed = ((mpu.baseAddr & 0x00FF) + mpu.Y) > 0x00FF;
    if (boundaryCrossed) {
        mpu.cycle++;
    } else {
        opcodes[mpu.opcode].dataHandler(mpu, value);
        mpu.cycle = 0;
        mpu.PC += 2;
    }
}
void fetchIndirectYDataNextPage(MPU& mpu) {
    uint8_t value = mpu.mem->read(mpu.baseAddr + mpu.Y);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.cycle = 0;
    mpu.PC += 2;
}
OpCode indirectYMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchIndirectYIndirectAddr, fetchIndirectYAddrLow, fetchIndirectYAddrHigh, fetchIndirectYData, fetchIndirectYDataNextPage, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

OpCode impliedSingleByte(void (*handler)(MPU&)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, handler, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
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

void opASL(MPU& mpu) {
    uint8_t oldA = mpu.A;
    mpu.A = mpu.A << 1;
    bool carry = 0x80 & oldA;
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z | MPU::Flag::C);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0);
    mpu.cycle = 0;
    mpu.PC++;
}

void opLSR(MPU& mpu) {
    uint8_t oldA = mpu.A;
    mpu.A = mpu.A >> 1;
    bool carry = 0x01 & oldA;
    bool zero = mpu.A == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z | MPU::Flag::C);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0);
    mpu.cycle = 0;
    mpu.PC++;
}

void opROL(MPU& mpu) {
    uint8_t oldA = mpu.A;
    mpu.A = (mpu.A << 1) | (mpu.P & MPU::Flag::C);
    bool carry = 0x80 & oldA;
    bool zero = mpu.A == 0;
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z | MPU::Flag::C);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0) | (neg ? MPU::Flag::N : 0);
    mpu.cycle = 0;
    mpu.PC++;
}

void opROR(MPU& mpu) {
    uint8_t oldA = mpu.A;
    mpu.A = (mpu.A >> 1) | ((mpu.P & MPU::Flag::C) << 7);
    bool carry = 0x01 & oldA;
    bool zero = mpu.A == 0;
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z | MPU::Flag::C);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0) | (neg ? MPU::Flag::N : 0);
    mpu.cycle = 0;
    mpu.PC++;
}

void opCLC(MPU& mpu) {
    mpu.P &= ~(MPU::Flag::C);
    mpu.cycle = 0;
    mpu.PC++;
}

void opCLD(MPU& mpu) {
    mpu.P &= ~(MPU::Flag::D);
    mpu.cycle = 0;
    mpu.PC++;
}

void opCLI(MPU& mpu) {
    mpu.P &= ~(MPU::Flag::I);
    mpu.cycle = 0;
    mpu.PC++;
}

void opCLV(MPU& mpu) {
    mpu.P &= ~(MPU::Flag::V);
    mpu.cycle = 0;
    mpu.PC++;
}

void opDEX(MPU& mpu) {
    mpu.X--;
    bool neg = static_cast<int8_t>(mpu.X) < 0;
    bool zero = mpu.X == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.cycle = 0;
    mpu.PC++;
}

void opDEY(MPU& mpu) {
    mpu.Y--;
    bool neg = static_cast<int8_t>(mpu.Y) < 0;
    bool zero = mpu.Y == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.cycle = 0;
    mpu.PC++;
}

void opINX(MPU& mpu) {
    mpu.X++;
    bool neg = static_cast<int8_t>(mpu.X) < 0;
    bool zero = mpu.X == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.cycle = 0;
    mpu.PC++;
}

void opINY(MPU& mpu) {
    mpu.Y++;
    bool neg = static_cast<int8_t>(mpu.Y) < 0;
    bool zero = mpu.Y == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.cycle = 0;
    mpu.PC++;
}

void opNOP(MPU& mpu) {
    mpu.cycle = 0;
    mpu.PC++;
}

void opSEC(MPU& mpu) {
    mpu.P |= MPU::Flag::C;
    mpu.cycle = 0;
    mpu.PC++;
}

void opSED(MPU& mpu) {
    mpu.P |= MPU::Flag::D;
    mpu.cycle = 0;
    mpu.PC++;
}

void opSEI(MPU& mpu) {
    mpu.P |= MPU::Flag::I;
    mpu.cycle = 0;
    mpu.PC++;
}

void opTAX(MPU& mpu) {
    mpu.X = mpu.A;
    bool neg = static_cast<int8_t>(mpu.X) < 0;
    bool zero = mpu.X == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.cycle = 0;
    mpu.PC++;
}

void opTAY(MPU& mpu) {
    mpu.Y = mpu.A;
    bool neg = static_cast<int8_t>(mpu.Y) < 0;
    bool zero = mpu.Y == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.cycle = 0;
    mpu.PC++;
}

void opTSX(MPU& mpu) {
    mpu.X = mpu.S;
    bool neg = static_cast<int8_t>(mpu.X) < 0;
    bool zero = mpu.X == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.cycle = 0;
    mpu.PC++;
}

void opTXA(MPU& mpu) {
    mpu.A = mpu.X;
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.cycle = 0;
    mpu.PC++;
}

void opTXS(MPU& mpu) {
    mpu.S = mpu.X;
    mpu.cycle = 0;
    mpu.PC++;
}

void opTYA(MPU& mpu) {
    mpu.A = mpu.Y;
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.cycle = 0;
    mpu.PC++;
}

// STA, STX, STY
void storeALen2(MPU& mpu) {
    mpu.mem->write(mpu.effectiveAddr, mpu.A);
    mpu.cycle = 0;
    mpu.PC += 2;
}
void storeXLen2(MPU& mpu) {
    mpu.mem->write(mpu.effectiveAddr, mpu.X);
    mpu.cycle = 0;
    mpu.PC += 2;
}
void storeYLen2(MPU& mpu) {
    mpu.mem->write(mpu.effectiveAddr, mpu.Y);
    mpu.cycle = 0;
    mpu.PC += 2;
}
void storeALen3(MPU& mpu) {
    mpu.mem->write(mpu.effectiveAddr, mpu.A);
    mpu.cycle = 0;
    mpu.PC += 3;
}
void storeXLen3(MPU& mpu) {
    mpu.mem->write(mpu.effectiveAddr, mpu.X);
    mpu.cycle = 0;
    mpu.PC += 3;
}
void storeYLen3(MPU& mpu) {
    mpu.mem->write(mpu.effectiveAddr, mpu.Y);
    mpu.cycle = 0;
    mpu.PC += 3;
}
void setEffectiveAddrAbsX(MPU& mpu) {
    mpu.effectiveAddr = mpu.baseAddr + mpu.X;
    mpu.cycle++;
}
void setEffectiveAddrAbsY(MPU& mpu) {
    mpu.effectiveAddr = mpu.baseAddr + mpu.Y;
    mpu.cycle++;
}
void storeAZeroPageX(MPU& mpu) {
    mpu.effectiveAddr = (mpu.baseAddr + mpu.X) & 0x00FF;
    mpu.mem->write(mpu.effectiveAddr, mpu.A);
    mpu.cycle = 0;
    mpu.PC += 2;
}
void storeYZeroPageX(MPU& mpu) {
    mpu.effectiveAddr = (mpu.baseAddr + mpu.X) & 0x00FF;
    mpu.mem->write(mpu.effectiveAddr, mpu.Y);
    mpu.cycle = 0;
    mpu.PC += 2;
}
void storeAZeroPageY(MPU& mpu) {
    mpu.effectiveAddr = (mpu.baseAddr + mpu.Y) & 0x00FF;
    mpu.mem->write(mpu.effectiveAddr, mpu.A);
    mpu.cycle = 0;
    mpu.PC += 2;
}
void storeXZeroPageY(MPU& mpu) {
    mpu.effectiveAddr = (mpu.baseAddr + mpu.Y) & 0x00FF;
    mpu.mem->write(mpu.effectiveAddr, mpu.X);
    mpu.cycle = 0;
    mpu.PC += 2;
}
void setEffectiveAddrIndY(MPU& mpu) {
    mpu.effectiveAddr = (mpu.baseAddr + mpu.Y) & 0x00FF;
    mpu.effectiveAddr |= mpu.baseAddr & 0xFF00;
    mpu.cycle++;
}
OpCode createSTAZeroPageOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageAddr, storeALen2, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTXZeroPageOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageAddr, storeXLen2, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTYZeroPageOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageAddr, storeYLen2, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTAAbsoluteOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr, storeALen3, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTXAbsoluteOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr, storeXLen2, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTYAbsoluteOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr, storeYLen2, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTAIndirectXOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchIndirectXBase, handlerNop, fetchIndirectXAddrLow, fetchIndirectXAddrHigh, storeALen2, undefinedOpcode };
    return opcodeData;
}
OpCode createSTAAbsoluteXOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteXAddrLow, fetchAbsoluteXAddrHigh, setEffectiveAddrAbsX, storeALen3, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTAAbsoluteYOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteYAddrLow, fetchAbsoluteYAddrHigh, setEffectiveAddrAbsY, storeALen3, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTAZeroPageXOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageXBase, handlerNop, storeAZeroPageX, undefinedOpcode, undefinedOpcode, undefinedOpcode  };
    return opcodeData;
}
OpCode createSTYZeroPageXOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageXBase, handlerNop, storeYZeroPageX, undefinedOpcode, undefinedOpcode, undefinedOpcode  };
    return opcodeData;
}
OpCode createSTAZeroPageYOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageXBase, handlerNop, storeAZeroPageY, undefinedOpcode, undefinedOpcode, undefinedOpcode  };
    return opcodeData;
}
OpCode createSTXZeroPageYOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageXBase, handlerNop, storeXZeroPageY, undefinedOpcode, undefinedOpcode, undefinedOpcode  };
    return opcodeData;
}
OpCode createSTAIndirectYOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchIndirectYIndirectAddr, fetchIndirectYAddrLow, fetchIndirectYAddrHigh, setEffectiveAddrIndY, storeALen2, undefinedOpcode };
    return opcodeData;
}

// Bit shift ops (ASL,LSR,ROL,ROR) and Inc/Dec
void opASLMod(MPU& mpu) {
    uint8_t oldVal = mpu.modVal;
    mpu.modVal = mpu.modVal << 1;
    bool carry = oldVal & 0x80;
    bool neg = static_cast<int8_t>(mpu.modVal) < 0;
    bool zero = mpu.modVal == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z | MPU::Flag::C);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0);
    mpu.cycle++;
}
void opLSRMod(MPU& mpu) {
    uint8_t oldVal = mpu.modVal;
    mpu.modVal = mpu.modVal >> 1;
    bool carry = 0x01 & oldVal;
    bool zero = mpu.modVal == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z | MPU::Flag::C);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0);
    mpu.cycle++;
}
void opROLMod(MPU& mpu) {
    uint8_t oldVal = mpu.modVal;
    mpu.modVal = (mpu.modVal << 1) | (mpu.P & MPU::Flag::C);
    bool carry = 0x80 & oldVal;
    bool zero = mpu.modVal == 0;
    bool neg = static_cast<int8_t>(mpu.modVal) < 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z | MPU::Flag::C);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0) | (neg ? MPU::Flag::N : 0);
    mpu.cycle++;
}
void opRORMod(MPU& mpu) {
    uint8_t oldVal = mpu.modVal;
    mpu.modVal = (mpu.modVal >> 1) | ((mpu.P & MPU::Flag::C) << 7);
    bool carry = 0x01 & oldVal;
    bool zero = mpu.modVal == 0;
    bool neg = static_cast<int8_t>(mpu.modVal) < 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z | MPU::Flag::C);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0) | (neg ? MPU::Flag::N : 0);
    mpu.cycle++;
}
void opINC(MPU& mpu) {
    mpu.modVal += 1;
    bool zero = mpu.modVal == 0;
    bool neg = static_cast<int8_t>(mpu.modVal) < 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (neg ? MPU::Flag::N : 0);
    mpu.cycle++;
}
void opDEC(MPU& mpu) {
    mpu.modVal -= 1;
    bool zero = mpu.modVal == 0;
    bool neg = static_cast<int8_t>(mpu.modVal) < 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (neg ? MPU::Flag::N : 0);
    mpu.cycle++;
}
void fetchModValZeroPage(MPU& mpu) {
    mpu.modVal = mpu.mem->read(mpu.effectiveAddr);
    mpu.cycle++;
}
void storeModValLen2(MPU& mpu) {
    mpu.mem->write(mpu.effectiveAddr, mpu.modVal);
    mpu.cycle = 0;
    mpu.PC += 2;
}
OpCode bitShiftZeroPage(void (*handler)(MPU&)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageAddr, fetchModValZeroPage, handler, storeModValLen2, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
void fetchModValAbsolute(MPU& mpu) {
    mpu.modVal = mpu.mem->read(mpu.effectiveAddr);
    mpu.cycle++;
}
void storeModValLen3(MPU& mpu) {
    mpu.mem->write(mpu.effectiveAddr, mpu.modVal);
    mpu.cycle = 0;
    mpu.PC += 3;
}
OpCode bitShiftAbsolute(void (*handler)(MPU& mpu)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr, fetchModValAbsolute, handler, storeModValLen3, undefinedOpcode };
    return opcodeData;
}
void fetchModValZeroPageX(MPU& mpu) {
    mpu.modVal = mpu.mem->read((mpu.baseAddr + mpu.X) & 0x00FF);
    mpu.cycle++;
}
OpCode bitShiftZeroPageX(void (*handler)(MPU& mpu)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageXBase, handlerNop, fetchModValZeroPageX, handler, storeModValLen2, undefinedOpcode };
    return opcodeData;
}
void fetchModValAbsoluteX(MPU& mpu) {
    mpu.modVal = mpu.mem->read(mpu.baseAddr + mpu.X);
    mpu.cycle++;
}
OpCode bitShiftAbsoluteX(void (*handler)(MPU& mpu)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteXAddrLow, fetchAbsoluteXAddrHigh, handlerNop, fetchModValAbsoluteX, handler, storeModValLen3 };
    return opcodeData;
}

// Branch Operations
void opBCC(MPU& mpu) {
    mpu.offset = mpu.mem->read(mpu.PC + 1);
    bool carry = MPU::Flag::C & 0x01;
    if (carry) {
        mpu.cycle = 0;
        mpu.PC += 2;
    } else {
        mpu.cycle++;
    }
}
void opBCS(MPU& mpu) {
    mpu.offset = mpu.mem->read(mpu.PC + 1);
    bool carry = MPU::Flag::C & 0x01;
    if (!carry) {
        mpu.cycle = 0;
        mpu.PC += 2;
    } else {
        mpu.cycle++;
    }
}
void opBEO(MPU& mpu) {
    mpu.offset = mpu.mem->read(mpu.PC + 1);
    bool zero = MPU::Flag::Z & 0x02;
    if (!zero) {
        mpu.cycle = 0;
        mpu.PC += 2;
    } else {
        mpu.cycle++;
    }
}
void opBNE(MPU& mpu) {
    mpu.offset = mpu.mem->read(mpu.PC + 1);
    bool zero = MPU::Flag::Z & 0x02;
    if (zero) {
        mpu.cycle = 0;
        mpu.PC += 2;
    } else {
        mpu.cycle++;
    }
}
void opBMI(MPU& mpu) {
    mpu.offset = mpu.mem->read(mpu.PC + 1);
    bool neg = MPU::Flag::N & 0x80;
    if (!neg) {
        mpu.cycle = 0;
        mpu.PC += 2;
    } else {
        mpu.cycle++;
    }
}
void opBPL(MPU& mpu) {
    mpu.offset = mpu.mem->read(mpu.PC + 1);
    bool neg = MPU::Flag::N & 0x80;
    if (neg) {
        mpu.cycle = 0;
        mpu.PC += 2;
    } else {
        mpu.cycle++;
    }
}
void opBVC(MPU& mpu) {
    mpu.offset = mpu.mem->read(mpu.PC + 1);
    bool overflow = MPU::Flag::V & 0x40;
    if (overflow) {
        mpu.cycle = 0;
        mpu.PC += 2;
    } else {
        mpu.cycle++;
    }
}
void opBVS(MPU& mpu) {
    mpu.offset = mpu.mem->read(mpu.PC + 1);
    bool overflow = MPU::Flag::V & 0x40;
    if (!overflow) {
        mpu.cycle = 0;
        mpu.PC += 2;
    } else {
        mpu.cycle++;
    }
}
void readBranchOpCode(MPU& mpu) {
    bool crossPageBoundary = (mpu.PC & 0x00FF) + 2 + mpu.offset > 0x00FF;
    if (!crossPageBoundary) {
        mpu.opcode = mpu.mem->read(mpu.PC + 2 + mpu.offset);
        mpu.cycle = 0;
        mpu.PC += 2;
    } else {
        mpu.cycle++;
    }
}
void readBranchBoundCrossOpCode(MPU& mpu) {
    mpu.opcode = mpu.mem->read(mpu.PC + 2 + mpu.offset + 1);
    mpu.cycle = 0;
    mpu.PC += 2;
}
OpCode branchOps(void (*handler)(MPU& mpu)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, handler, readBranchOpCode, readBranchBoundCrossOpCode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}

// BRK instruction (called for hardware interrupt)
void brkPushPCH(MPU& mpu) {
    mpu.mem->write(0x0100 | mpu.S, mpu.PC & 0xFF);
    mpu.S--;
    mpu.cycle++;
}
void brkPushPCL(MPU& mpu) {
    mpu.mem->write(0x0100 | mpu.S, (mpu.PC >> 8) & 0xFF);
    mpu.S--;
    mpu.cycle++;
}
void brkPushP(MPU& mpu) {
    mpu.mem->write(0x0100 | mpu.S, mpu.P);
    mpu.S--;
    mpu.cycle++;
}
void brkFetchAddrLow(MPU& mpu) {
    mpu.PC = mpu.mem->read(mpu.handlingNMI ? 0xFFFA : 0xFFFE);
    mpu.cycle++;
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
    mpu.cycle++;
}
void rtiPullPCL(MPU& mpu) {
    mpu.S--;
    mpu.PC = mpu.mem->read(0x0100 | mpu.S);
    mpu.cycle++;
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
    opcodes[0x7D] = absoluteXMode(opADC);
    opcodes[0x79] = absoluteYMode(opADC);
    opcodes[0x75] = zeroPageXMode(opADC);
    opcodes[0x71] = indirectYMode(opADC);
    opcodes[0x29] = immediateMode(opAND);
    opcodes[0x2D] = absoluteMode(opAND);
    opcodes[0x25] = zeroPageMode(opAND);
    opcodes[0x21] = indirectXMode(opAND);
    opcodes[0x3D] = absoluteXMode(opAND);
    opcodes[0x39] = absoluteYMode(opAND);
    opcodes[0x35] = zeroPageXMode(opAND);
    opcodes[0x31] = indirectYMode(opAND);
    opcodes[0x2C] = absoluteMode(opBIT);
    opcodes[0x24] = zeroPageMode(opBIT);
    opcodes[0xC9] = immediateMode(opCMP);
    opcodes[0xCD] = absoluteMode(opCMP);
    opcodes[0xC5] = zeroPageMode(opCMP);
    opcodes[0xC1] = indirectXMode(opCMP);
    opcodes[0xDD] = absoluteXMode(opCMP);
    opcodes[0xD9] = absoluteYMode(opCMP);
    opcodes[0xD5] = zeroPageXMode(opCMP);
    opcodes[0xD1] = indirectYMode(opCMP);
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
    opcodes[0x5D] = absoluteXMode(opEOR);
    opcodes[0x59] = absoluteYMode(opEOR);
    opcodes[0x55] = zeroPageXMode(opEOR);
    opcodes[0x51] = indirectYMode(opEOR);
    opcodes[0xA9] = immediateMode(opLDA);
    opcodes[0xAD] = absoluteMode(opLDA);
    opcodes[0xA5] = zeroPageMode(opLDA);
    opcodes[0xA1] = indirectXMode(opLDA);
    opcodes[0xBD] = absoluteXMode(opLDA);
    opcodes[0xB9] = absoluteYMode(opLDA);
    opcodes[0xB5] = zeroPageXMode(opLDA);
    opcodes[0xB1] = indirectYMode(opLDA);
    opcodes[0xA2] = immediateMode(opLDX);
    opcodes[0xAE] = absoluteMode(opLDX);
    opcodes[0xA6] = zeroPageMode(opLDX);
    opcodes[0xBE] = absoluteYMode(opLDX);
    opcodes[0xB6] = zeroPageYMode(opLDX);
    opcodes[0xA0] = immediateMode(opLDY);
    opcodes[0xAC] = absoluteMode(opLDY);
    opcodes[0xA4] = zeroPageMode(opLDY);
    opcodes[0xBC] = absoluteXMode(opLDY);
    opcodes[0xB4] = zeroPageXMode(opLDY);
    opcodes[0x09] = immediateMode(opORA);
    opcodes[0x0D] = absoluteMode(opORA);
    opcodes[0x05] = zeroPageMode(opORA);
    opcodes[0x01] = indirectXMode(opORA);
    opcodes[0x1D] = absoluteXMode(opORA);
    opcodes[0x19] = absoluteYMode(opORA);
    opcodes[0x15] = zeroPageXMode(opORA);
    opcodes[0x11] = indirectYMode(opORA);
    opcodes[0xE9] = immediateMode(opSBC);
    opcodes[0xED] = absoluteMode(opSBC);
    opcodes[0xE5] = zeroPageMode(opSBC);
    opcodes[0xE1] = indirectXMode(opSBC);
    opcodes[0xFD] = absoluteXMode(opSBC);
    opcodes[0xF9] = absoluteYMode(opSBC);
    opcodes[0xF5] = zeroPageXMode(opSBC);
    opcodes[0xF1] = indirectYMode(opSBC);

    // Single Byte Instructions
    opcodes[0x0A] = impliedSingleByte(opASL);
    opcodes[0x18] = impliedSingleByte(opCLC);
    opcodes[0xD8] = impliedSingleByte(opCLD);
    opcodes[0x58] = impliedSingleByte(opCLI);
    opcodes[0xB8] = impliedSingleByte(opCLV);
    opcodes[0xCA] = impliedSingleByte(opDEX);
    opcodes[0xB8] = impliedSingleByte(opDEY);
    opcodes[0xE8] = impliedSingleByte(opINX);
    opcodes[0xC8] = impliedSingleByte(opINY);
    opcodes[0x4A] = impliedSingleByte(opLSR);
    opcodes[0xEA] = impliedSingleByte(opNOP);
    opcodes[0x2A] = impliedSingleByte(opROL);
    opcodes[0x6A] = impliedSingleByte(opROR);
    opcodes[0x38] = impliedSingleByte(opSEC);
    opcodes[0xF8] = impliedSingleByte(opSED);
    opcodes[0x78] = impliedSingleByte(opSEI);
    opcodes[0xAA] = impliedSingleByte(opTAX);
    opcodes[0xA8] = impliedSingleByte(opTAY);
    opcodes[0xBA] = impliedSingleByte(opTSX);
    opcodes[0x8A] = impliedSingleByte(opTXA);
    opcodes[0x9A] = impliedSingleByte(opTXS);
    opcodes[0x98] = impliedSingleByte(opTYA);

    // store operations
    opcodes[0x8D] = createSTAAbsoluteOpCode();
    opcodes[0x8E] = createSTXAbsoluteOpCode();
    opcodes[0x8C] = createSTYAbsoluteOpCode();
    opcodes[0x85] = createSTAZeroPageOpCode();
    opcodes[0x86] = createSTXZeroPageOpCode();
    opcodes[0x84] = createSTYZeroPageOpCode();
    opcodes[0x81] = createSTAIndirectXOpCode();
    opcodes[0x95] = createSTAZeroPageXOpCode();
    opcodes[0x94] = createSTYZeroPageXOpCode();
    opcodes[0x9D] = createSTAAbsoluteXOpCode();
    opcodes[0x99] = createSTAAbsoluteYOpCode();
    opcodes[0x96] = createSTXZeroPageYOpCode();
    opcodes[0x91] = createSTAIndirectYOpCode();

    // Read - Modify - Write Operations (Bit shifts and Inc/Dec)
    opcodes[0x0E] = bitShiftAbsolute(opASLMod);
    opcodes[0x06] = bitShiftZeroPage(opASLMod);
    opcodes[0x16] = bitShiftZeroPageX(opASLMod);
    opcodes[0x1E] = bitShiftAbsoluteX(opASLMod);
    opcodes[0xCE] = bitShiftAbsolute(opDEC);
    opcodes[0xC6] = bitShiftZeroPage(opDEC);
    opcodes[0xD6] = bitShiftZeroPageX(opDEC);
    opcodes[0xDE] = bitShiftAbsoluteX(opDEC);
    opcodes[0xEE] = bitShiftAbsolute(opINC);
    opcodes[0xE6] = bitShiftZeroPage(opINC);
    opcodes[0xF6] = bitShiftZeroPageX(opINC);
    opcodes[0xFE] = bitShiftAbsoluteX(opINC);
    opcodes[0x4E] = bitShiftAbsolute(opLSR);
    opcodes[0x46] = bitShiftZeroPage(opLSR);
    opcodes[0x56] = bitShiftZeroPageX(opLSR);
    opcodes[0x5E] = bitShiftAbsoluteX(opLSR);
    opcodes[0x2E] = bitShiftAbsolute(opROL);
    opcodes[0x26] = bitShiftZeroPage(opROL);
    opcodes[0x36] = bitShiftZeroPageX(opROL);
    opcodes[0x3E] = bitShiftAbsoluteX(opROL);
    opcodes[0x6E] = bitShiftAbsolute(opROR);
    opcodes[0x66] = bitShiftZeroPage(opROR);
    opcodes[0x76] = bitShiftZeroPageX(opROR);
    opcodes[0x7E] = bitShiftAbsoluteX(opROR);

    // Branch Operations
    opcodes[0x90] = branchOps(opBCC);
    opcodes[0xB0] = branchOps(opBCS);
    opcodes[0xF0] = branchOps(opBEO);
    opcodes[0x30] = branchOps(opBMI);
    opcodes[0xD0] = branchOps(opBNE);
    opcodes[0x10] = branchOps(opBPL);
    opcodes[0x50] = branchOps(opBVC);
    opcodes[0x70] = branchOps(opBVS);


    // misc operations
    opcodes[0x00] = createBRKOpCode();
    opcodes[0x40] = createRTIOpCode();

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
