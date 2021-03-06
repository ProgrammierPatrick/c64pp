#include "mpu.h"

#include <array>
#include <stdexcept>

uint8_t readMem(MPU& mpu, uint16_t addr) {
    mpu.lastMemWritten = false;
    return mpu.mem->read(addr);
}
void writeMem(MPU& mpu, uint16_t addr, uint8_t data) {
    mpu.lastMemWritten = true;
    mpu.mem->write(addr, data);
}

void dataHandlerNop(MPU& mpu, uint8_t data) {
    throw std::runtime_error("[MPU] DataHandlerNop() called!");
}

struct OpCode {
    // int numBytes; // length of istruction including opcode
    std::array<void (*)(MPU&), 8> handlers;
    void (*dataHandler)(MPU& mpu, uint8_t data) = dataHandlerNop;
};
extern const std::array<OpCode, 256> opcodes;

void undefinedOpcode(MPU& mpu) {
    throw std::runtime_error("[MPU] Undefined OpCode encountered!");
    mpu.T = 0;
}

void fetchOpCode(MPU& mpu) {
    mpu.opcode = readMem(mpu, mpu.PC);
    mpu.T++;
}

void handlerNop(MPU& mpu) {
    mpu.T++;
}

void fetchImmediate(MPU& mpu) {
    uint8_t value = readMem(mpu, mpu.PC + 1);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.T = 0;
    mpu.PC += 2;
}
OpCode immediateMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    // opcodeData.numBytes = 2;
    opcodeData.handlers = { fetchOpCode, fetchImmediate, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchAbsoluteLowAddr(MPU& mpu) {
    mpu.effectiveAddr = readMem(mpu, mpu.PC + 1);
    mpu.T++;
}
void fetchAbsoluteHighAddr(MPU& mpu) {
    mpu.effectiveAddr |= readMem(mpu, mpu.PC + 2) << 8;
    mpu.T++;
}
void fetchAbsoluteData(MPU& mpu) {
    uint8_t value = readMem(mpu, mpu.effectiveAddr);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.T = 0;
    mpu.PC += 3;
}
OpCode absoluteMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr, fetchAbsoluteData, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchZeroPageAddr(MPU& mpu) {
    mpu.effectiveAddr = readMem(mpu, mpu.PC + 1);
    mpu.T++;
}
void fetchZeroPageData(MPU& mpu) {
    uint8_t value = readMem(mpu, mpu.effectiveAddr);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.T = 0;
    mpu.PC += 2;
}
OpCode zeroPageMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageAddr, fetchZeroPageData, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchIndirectXBase(MPU& mpu) {
    mpu.baseAddr = readMem(mpu, mpu.PC + 1);
    mpu.T++;
}
void fetchIndirectXAddrLow(MPU& mpu) {
    mpu.effectiveAddr = readMem(mpu, (mpu.baseAddr + mpu.X) & 0x00FF);
    mpu.T++;
}
void fetchIndirectXAddrHigh(MPU& mpu) {
    mpu.effectiveAddr |= readMem(mpu, (mpu.baseAddr + mpu.X + 1) & 0x00FF) << 8;
    mpu.T++;
}
void fetchIndirectXData(MPU& mpu) {
    uint8_t value = readMem(mpu, mpu.effectiveAddr);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.T = 0;
    mpu.PC += 2;
}
OpCode indirectXMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchIndirectXBase, handlerNop, fetchIndirectXAddrLow, fetchIndirectXAddrHigh, fetchIndirectXData, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchAbsoluteXAddrLow(MPU& mpu) {
    mpu.baseAddr = readMem(mpu, mpu.PC + 1);
    mpu.T++;
}
void fetchAbsoluteXAddrHigh(MPU& mpu) {
    mpu.baseAddr |= readMem(mpu, mpu.PC + 2) << 8;
    mpu.T++;
}
void fetchAbsoluteXData(MPU& mpu) {
    uint8_t value = readMem(mpu, mpu.baseAddr + mpu.X);
    bool boundaryCrossed = (mpu.baseAddr & 0x00FF) + mpu.X > 0x00FF;
    if (boundaryCrossed) {
        mpu.T++;
    } else {
        opcodes[mpu.opcode].dataHandler(mpu, value);
        mpu.T = 0;
        mpu.PC += 3;
    }
}
void fetchAbsoluteXDataNextPage(MPU& mpu) {
    uint8_t value = readMem(mpu, mpu.baseAddr + mpu.X);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.T = 0;
    mpu.PC += 3;
}
OpCode absoluteXMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteXAddrLow, fetchAbsoluteXAddrHigh, fetchAbsoluteXData, fetchAbsoluteXDataNextPage, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchAbsoluteYAddrLow(MPU& mpu) {
    mpu.baseAddr = readMem(mpu, mpu.PC + 1);
    mpu.T++;
}
void fetchAbsoluteYAddrHigh(MPU& mpu) {
    mpu.baseAddr |= readMem(mpu, mpu.PC + 2) << 8;
    mpu.T++;
}
void fetchAbsoluteYData(MPU& mpu) {
    uint8_t value = readMem(mpu, mpu.baseAddr + mpu.Y);
    bool boundaryCrossed = (mpu.baseAddr & 0x00FF) + mpu.Y > 0x00FF;
    if (boundaryCrossed) {
        mpu.T++;
    } else {
        opcodes[mpu.opcode].dataHandler(mpu, value);
        mpu.T = 0;
        mpu.PC += 3;
    }
}
void fetchAbsoluteYDataNextPage(MPU& mpu) {
    uint8_t value = readMem(mpu, mpu.baseAddr + mpu.Y);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.T = 0;
    mpu.PC += 3;
}
OpCode absoluteYMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteYAddrLow, fetchAbsoluteYAddrHigh, fetchAbsoluteYData, fetchAbsoluteYDataNextPage, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchZeroPageXBase(MPU& mpu) {
    mpu.baseAddr = readMem(mpu, mpu.PC + 1);
    mpu.T++;
}
void fetchZeroPageXData(MPU& mpu) {
    uint8_t value = readMem(mpu, (mpu.baseAddr + mpu.X) & 0x00FF);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.T = 0;
    mpu.PC += 2;
}
OpCode zeroPageXMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageXBase, handlerNop, fetchZeroPageXData, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchZeroPageYBase(MPU& mpu) {
    mpu.baseAddr = readMem(mpu, mpu.PC + 1);
    mpu.T++;
}
void fetchZeroPageYData(MPU& mpu) {
    uint8_t value = readMem(mpu, (mpu.baseAddr + mpu.Y) & 0x00FF);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.T = 0;
    mpu.PC += 2;
}
OpCode zeroPageYMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageYBase, handlerNop, fetchZeroPageYData, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void fetchIndirectYIndirectAddr(MPU& mpu) {
    mpu.indirectAddr = readMem(mpu, mpu.PC + 1);
    mpu.T++;
}
void fetchIndirectYAddrLow(MPU& mpu) {
    mpu.baseAddr = readMem(mpu, mpu.indirectAddr);
    mpu.T++;
}
void fetchIndirectYAddrHigh(MPU& mpu) {
    mpu.baseAddr |= readMem(mpu, (mpu.indirectAddr + 1) & 0x00FF) << 8;
    mpu.T++;
}
void fetchIndirectYData(MPU& mpu) {
    uint8_t value = readMem(mpu, mpu.baseAddr + mpu.Y);
    bool boundaryCrossed = ((mpu.baseAddr & 0x00FF) + mpu.Y) > 0x00FF;
    if (boundaryCrossed) {
        mpu.T++;
    } else {
        opcodes[mpu.opcode].dataHandler(mpu, value);
        mpu.T = 0;
        mpu.PC += 2;
    }
}
void fetchIndirectYDataNextPage(MPU& mpu) {
    uint8_t value = readMem(mpu, mpu.baseAddr + mpu.Y);
    opcodes[mpu.opcode].dataHandler(mpu, value);
    mpu.T = 0;
    mpu.PC += 2;
}
OpCode indirectYMode(void (*handler)(MPU&,uint8_t)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchIndirectYIndirectAddr, fetchIndirectYAddrLow, fetchIndirectYAddrHigh, fetchIndirectYData, fetchIndirectYDataNextPage, undefinedOpcode, undefinedOpcode };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

OpCode impliedSingleByte(void (*handler)(MPU&)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, handler, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}

void opADC(MPU& mpu, uint8_t value) {
    uint8_t oldA = mpu.A;
    mpu.A = oldA + value + (mpu.P & MPU::Flag::C);
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    bool carry = (oldA + static_cast<uint16_t>(value) + (mpu.P & MPU::Flag::C)) > 0x00FF;
    bool overflow = (oldA ^ mpu.A) & (value ^ mpu.A) & 0x80; // https://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
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
    bool zero = (mpu.A & value) == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::V | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (overflow ? MPU::Flag::V : 0) |(zero ? MPU::Flag::Z : 0);
}

void opCMP(MPU& mpu, uint8_t value) {
    int8_t diff = mpu.A - value;
    bool neg = diff & 0x80;
    bool zero = diff == 0;
    bool carry = static_cast<int16_t>(mpu.A) - static_cast<int16_t>(value) >= 0;
    mpu.P &= ~(MPU::Flag::C | MPU::Flag::Z | MPU::Flag::N);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (carry ? MPU::Flag::C : 0) |(zero ? MPU::Flag::Z : 0);
}

void opCPX(MPU& mpu, uint8_t value) {
    int8_t diff = mpu.X - value;
    bool neg = diff & 0x80;
    bool zero = diff == 0;
    bool carry = static_cast<int16_t>(mpu.X) - static_cast<int16_t>(value) >= 0;
    mpu.P &= ~(MPU::Flag::C | MPU::Flag::Z | MPU::Flag::N);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (carry ? MPU::Flag::C : 0) |(zero ? MPU::Flag::Z : 0);
}

void opCPY(MPU& mpu, uint8_t value) {
    int8_t diff = mpu.Y - value;
    bool neg = diff & 0x80;
    bool zero = diff == 0;
    bool carry = static_cast<int16_t>(mpu.Y) - static_cast<int16_t>(value) >= 0;
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
    bool carry = static_cast<int16_t>(oldA) - static_cast<int16_t>(value) - (0x01 ^ (mpu.P & MPU::Flag::C)) >= 0;
    bool overflow = (oldA ^ mpu.A) & ~(value ^ mpu.A) & 0x80; // https://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
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
    mpu.T = 0;
    mpu.PC++;
}

void opLSR(MPU& mpu) {
    uint8_t oldA = mpu.A;
    mpu.A = mpu.A >> 1;
    bool carry = 0x01 & oldA;
    bool zero = mpu.A == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z | MPU::Flag::C);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0);
    mpu.T = 0;
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
    mpu.T = 0;
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
    mpu.T = 0;
    mpu.PC++;
}

void opCLC(MPU& mpu) {
    mpu.P &= ~(MPU::Flag::C);
    mpu.T = 0;
    mpu.PC++;
}

void opCLD(MPU& mpu) {
    mpu.P &= ~(MPU::Flag::D);
    mpu.T = 0;
    mpu.PC++;
}

void opCLI(MPU& mpu) {
    mpu.P &= ~(MPU::Flag::I);
    mpu.T = 0;
    mpu.PC++;
}

void opCLV(MPU& mpu) {
    mpu.P &= ~(MPU::Flag::V);
    mpu.T = 0;
    mpu.PC++;
}

void opDEX(MPU& mpu) {
    mpu.X--;
    bool neg = static_cast<int8_t>(mpu.X) < 0;
    bool zero = mpu.X == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.T = 0;
    mpu.PC++;
}

void opDEY(MPU& mpu) {
    mpu.Y--;
    bool neg = static_cast<int8_t>(mpu.Y) < 0;
    bool zero = mpu.Y == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.T = 0;
    mpu.PC++;
}

void opINX(MPU& mpu) {
    mpu.X++;
    bool neg = static_cast<int8_t>(mpu.X) < 0;
    bool zero = mpu.X == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.T = 0;
    mpu.PC++;
}

void opINY(MPU& mpu) {
    mpu.Y++;
    bool neg = static_cast<int8_t>(mpu.Y) < 0;
    bool zero = mpu.Y == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.T = 0;
    mpu.PC++;
}

void opNOP(MPU& mpu) {
    mpu.T = 0;
    mpu.PC++;
}

void opSEC(MPU& mpu) {
    mpu.P |= MPU::Flag::C;
    mpu.T = 0;
    mpu.PC++;
}

void opSED(MPU& mpu) {
    mpu.P |= MPU::Flag::D;
    mpu.T = 0;
    mpu.PC++;
}

void opSEI(MPU& mpu) {
    mpu.P |= MPU::Flag::I;
    mpu.T = 0;
    mpu.PC++;
}

void opTAX(MPU& mpu) {
    mpu.X = mpu.A;
    bool neg = static_cast<int8_t>(mpu.X) < 0;
    bool zero = mpu.X == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.T = 0;
    mpu.PC++;
}

void opTAY(MPU& mpu) {
    mpu.Y = mpu.A;
    bool neg = static_cast<int8_t>(mpu.Y) < 0;
    bool zero = mpu.Y == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.T = 0;
    mpu.PC++;
}

void opTSX(MPU& mpu) {
    mpu.X = mpu.S;
    bool neg = static_cast<int8_t>(mpu.X) < 0;
    bool zero = mpu.X == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.T = 0;
    mpu.PC++;
}

void opTXA(MPU& mpu) {
    mpu.A = mpu.X;
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.T = 0;
    mpu.PC++;
}

void opTXS(MPU& mpu) {
    mpu.S = mpu.X;
    mpu.T = 0;
    mpu.PC++;
}

void opTYA(MPU& mpu) {
    mpu.A = mpu.Y;
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.T = 0;
    mpu.PC++;
}

// STA, STX, STY
void storeALen2(MPU& mpu) {
    writeMem(mpu, mpu.effectiveAddr, mpu.A);
    mpu.T = 0;
    mpu.PC += 2;
}
void storeXLen2(MPU& mpu) {
    writeMem(mpu, mpu.effectiveAddr, mpu.X);
    mpu.T = 0;
    mpu.PC += 2;
}
void storeYLen2(MPU& mpu) {
    writeMem(mpu, mpu.effectiveAddr, mpu.Y);
    mpu.T = 0;
    mpu.PC += 2;
}
void storeALen3(MPU& mpu) {
    writeMem(mpu, mpu.effectiveAddr, mpu.A);
    mpu.T = 0;
    mpu.PC += 3;
}
void storeXLen3(MPU& mpu) {
    writeMem(mpu, mpu.effectiveAddr, mpu.X);
    mpu.T = 0;
    mpu.PC += 3;
}
void storeYLen3(MPU& mpu) {
    writeMem(mpu, mpu.effectiveAddr, mpu.Y);
    mpu.T = 0;
    mpu.PC += 3;
}
void setEffectiveAddrAbsX(MPU& mpu) {
    mpu.effectiveAddr = mpu.baseAddr + mpu.X;

    // 6502 bug: ghost read at base addr +X
    readMem(mpu, mpu.baseAddr);

    mpu.T++;
}
void setEffectiveAddrAbsY(MPU& mpu) {
    mpu.effectiveAddr = mpu.baseAddr + mpu.Y;

    // 6502 bug: ghost read at base addr +X
    readMem(mpu, mpu.baseAddr);

    mpu.T++;
}
void storeAZeroPageX(MPU& mpu) {
    mpu.effectiveAddr = (mpu.baseAddr + mpu.X) & 0x00FF;
    writeMem(mpu, mpu.effectiveAddr, mpu.A);
    mpu.T = 0;
    mpu.PC += 2;
}
void storeYZeroPageX(MPU& mpu) {
    mpu.effectiveAddr = (mpu.baseAddr + mpu.X) & 0x00FF;
    writeMem(mpu, mpu.effectiveAddr, mpu.Y);
    mpu.T = 0;
    mpu.PC += 2;
}
void storeAZeroPageY(MPU& mpu) {
    mpu.effectiveAddr = (mpu.baseAddr + mpu.Y) & 0x00FF;
    writeMem(mpu, mpu.effectiveAddr, mpu.A);
    mpu.T = 0;
    mpu.PC += 2;
}
void storeXZeroPageY(MPU& mpu) {
    mpu.effectiveAddr = (mpu.baseAddr + mpu.Y) & 0x00FF;
    writeMem(mpu, mpu.effectiveAddr, mpu.X);
    mpu.T = 0;
    mpu.PC += 2;
}
void setEffectiveAddrIndY(MPU& mpu) {
    mpu.effectiveAddr = mpu.baseAddr + mpu.Y;
    mpu.T++;
}
OpCode createSTAZeroPageOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageAddr, storeALen2, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTXZeroPageOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageAddr, storeXLen2, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTYZeroPageOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageAddr, storeYLen2, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTAAbsoluteOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr, storeALen3, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTXAbsoluteOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr, storeXLen3, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTYAbsoluteOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr, storeYLen3, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTAIndirectXOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchIndirectXBase, handlerNop, fetchIndirectXAddrLow, fetchIndirectXAddrHigh, storeALen2, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTAAbsoluteXOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteXAddrLow, fetchAbsoluteXAddrHigh, setEffectiveAddrAbsX, storeALen3, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTAAbsoluteYOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteYAddrLow, fetchAbsoluteYAddrHigh, setEffectiveAddrAbsY, storeALen3, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTAZeroPageXOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageXBase, handlerNop, storeAZeroPageX, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTYZeroPageXOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageXBase, handlerNop, storeYZeroPageX, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTAZeroPageYOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageXBase, handlerNop, storeAZeroPageY, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTXZeroPageYOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageXBase, handlerNop, storeXZeroPageY, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createSTAIndirectYOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchIndirectYIndirectAddr, fetchIndirectYAddrLow, fetchIndirectYAddrHigh, setEffectiveAddrIndY, storeALen2, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}

// Bit shift ops (ASL,LSR,ROL,ROR) and Inc/Dec
void opASLMod(MPU& mpu) {
    // 6502 bug: double store at RMW
    writeMem(mpu, mpu.effectiveAddr, mpu.modVal);

    uint8_t oldVal = mpu.modVal;
    mpu.modVal = mpu.modVal << 1;
    bool carry = oldVal & 0x80;
    bool neg = static_cast<int8_t>(mpu.modVal) < 0;
    bool zero = mpu.modVal == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z | MPU::Flag::C);
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0);
    mpu.T++;
}
void opLSRMod(MPU& mpu) {
    // 6502 bug: double store at RMW
    writeMem(mpu, mpu.effectiveAddr, mpu.modVal);

    uint8_t oldVal = mpu.modVal;
    mpu.modVal = mpu.modVal >> 1;
    bool carry = 0x01 & oldVal;
    bool zero = mpu.modVal == 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z | MPU::Flag::C);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0);
    mpu.T++;
}
void opROLMod(MPU& mpu) {
    // 6502 bug: double store at RMW
    writeMem(mpu, mpu.effectiveAddr, mpu.modVal);

    uint8_t oldVal = mpu.modVal;
    mpu.modVal = (mpu.modVal << 1) | (mpu.P & MPU::Flag::C);
    bool carry = 0x80 & oldVal;
    bool zero = mpu.modVal == 0;
    bool neg = static_cast<int8_t>(mpu.modVal) < 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z | MPU::Flag::C);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0) | (neg ? MPU::Flag::N : 0);
    mpu.T++;
}
void opRORMod(MPU& mpu) {
    // 6502 bug: double store at RMW
    writeMem(mpu, mpu.effectiveAddr, mpu.modVal);

    uint8_t oldVal = mpu.modVal;
    mpu.modVal = (mpu.modVal >> 1) | ((mpu.P & MPU::Flag::C) << 7);
    bool carry = 0x01 & oldVal;
    bool zero = mpu.modVal == 0;
    bool neg = static_cast<int8_t>(mpu.modVal) < 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z | MPU::Flag::C);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0) | (neg ? MPU::Flag::N : 0);
    mpu.T++;
}
void opINC(MPU& mpu) {
    // 6502 bug: double store at RMW
    writeMem(mpu, mpu.effectiveAddr, mpu.modVal);

    mpu.modVal += 1;
    bool zero = mpu.modVal == 0;
    bool neg = static_cast<int8_t>(mpu.modVal) < 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (neg ? MPU::Flag::N : 0);
    mpu.T++;
}
void opDEC(MPU& mpu) {
    // 6502 bug: double store at RMW
    writeMem(mpu, mpu.effectiveAddr, mpu.modVal);

    mpu.modVal -= 1;
    bool zero = mpu.modVal == 0;
    bool neg = static_cast<int8_t>(mpu.modVal) < 0;
    mpu.P &= ~(MPU::Flag::N | MPU::Flag::Z);
    mpu.P |= (zero ? MPU::Flag::Z : 0) | (neg ? MPU::Flag::N : 0);
    mpu.T++;
}
void fetchModValZeroPage(MPU& mpu) {
    mpu.modVal = readMem(mpu, mpu.effectiveAddr);
    mpu.T++;
}
void storeModValLen2(MPU& mpu) {
    writeMem(mpu, mpu.effectiveAddr, mpu.modVal);
    mpu.T = 0;
    mpu.PC += 2;
}
OpCode bitShiftZeroPage(void (*handler)(MPU&)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageAddr, fetchModValZeroPage, handler, storeModValLen2, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
void fetchModValAbsolute(MPU& mpu) {
    mpu.modVal = readMem(mpu, mpu.effectiveAddr);
    mpu.T++;
}
void storeModValLen3(MPU& mpu) {
    writeMem(mpu, mpu.effectiveAddr, mpu.modVal);
    mpu.T = 0;
    mpu.PC += 3;
}
OpCode bitShiftAbsolute(void (*handler)(MPU& mpu)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr, fetchModValAbsolute, handler, storeModValLen3, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
void fetchModValZeroPageX(MPU& mpu) {
    mpu.effectiveAddr = (mpu.baseAddr + mpu.X) & 0x00FF;
    mpu.modVal = readMem(mpu, mpu.effectiveAddr);
    mpu.T++;
}
OpCode bitShiftZeroPageX(void (*handler)(MPU& mpu)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchZeroPageXBase, handlerNop, fetchModValZeroPageX, handler, storeModValLen2, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
void fetchModValAbsoluteX(MPU& mpu) {
    mpu.effectiveAddr = mpu.baseAddr + mpu.X;
    mpu.modVal = readMem(mpu, mpu.effectiveAddr);
    mpu.T++;
}
OpCode bitShiftAbsoluteX(void (*handler)(MPU& mpu)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteXAddrLow, fetchAbsoluteXAddrHigh, handlerNop, fetchModValAbsoluteX, handler, storeModValLen3, undefinedOpcode };
    return opcodeData;
}

// Branch Operations
void opBCC(MPU& mpu) {
    mpu.offset = readMem(mpu, mpu.PC + 1);
    bool carry = mpu.P & MPU::Flag::C;
    if (carry) {
        mpu.T = 0;
        mpu.PC += 2;
    } else {
        mpu.T++;
    }
}
void opBCS(MPU& mpu) {
    mpu.offset = readMem(mpu, mpu.PC + 1);
    bool carry = mpu.P & MPU::Flag::C;
    if (!carry) {
        mpu.T = 0;
        mpu.PC += 2;
    } else {
        mpu.T++;
    }
}
void opBEQ(MPU& mpu) {
    mpu.offset = readMem(mpu, mpu.PC + 1);
    bool zero = mpu.P & MPU::Flag::Z;
    if (!zero) {
        mpu.T = 0;
        mpu.PC += 2;
    } else {
        mpu.T++;
    }
}
void opBNE(MPU& mpu) {
    mpu.offset = readMem(mpu, mpu.PC + 1);
    bool zero = mpu.P & MPU::Flag::Z;
    if (zero) {
        mpu.T = 0;
        mpu.PC += 2;
    } else {
        mpu.T++;
    }
}
void opBMI(MPU& mpu) {
    mpu.offset = readMem(mpu, mpu.PC + 1);
    bool neg = mpu.P & MPU::Flag::N;
    if (!neg) {
        mpu.T = 0;
        mpu.PC += 2;
    } else {
        mpu.T++;
    }
}
void opBPL(MPU& mpu) {
    mpu.offset = readMem(mpu, mpu.PC + 1);
    bool neg = mpu.P & MPU::Flag::N;
    if (neg) {
        mpu.T = 0;
        mpu.PC += 2;
    } else {
        mpu.T++;
    }
}
void opBVC(MPU& mpu) {
    mpu.offset = readMem(mpu, mpu.PC + 1);
    bool overflow = mpu.P & MPU::Flag::V;
    if (overflow) {
        mpu.T = 0;
        mpu.PC += 2;
    } else {
        mpu.T++;
    }
}
void opBVS(MPU& mpu) {
    mpu.offset = readMem(mpu, mpu.PC + 1);
    bool overflow = mpu.P & MPU::Flag::V;
    if (!overflow) {
        mpu.T = 0;
        mpu.PC += 2;
    } else {
        mpu.T++;
    }
}
void readBranchOpCode(MPU& mpu) {
    bool crossPageBoundary = (mpu.PC & 0x00FF) + 2 + mpu.offset > 0x00FF;
    if (!crossPageBoundary) {
        mpu.T = 0;
        mpu.PC += 2 + mpu.offset;
    } else {
        mpu.T++;
    }
}
void readBranchBoundCrossOpCode(MPU& mpu) {
    mpu.T = 0;
    mpu.PC += 2 + mpu.offset;
}
OpCode branchOps(void (*handler)(MPU& mpu)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, handler, readBranchOpCode, readBranchBoundCrossOpCode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}

// Push / Pull Instructions
void opPHA(MPU& mpu) {
    writeMem(mpu, 0x0100 | mpu.S, mpu.A);
    mpu.S--;
    mpu.T = 0;
    mpu.PC += 1;
}
void opPHP(MPU& mpu) {
    writeMem(mpu, 0x0100 | mpu.S, mpu.P | MPU::Flag::B | 0x20); // B and empty flag are always 1 when read
    mpu.S--;
    mpu.T = 0;
    mpu.PC += 1;
}
OpCode pushOperation(void (*handler)(MPU& mpu)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, handlerNop, handler, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
void opPLA(MPU& mpu) {
    mpu.S++;
    mpu.A = readMem(mpu, 0x0100 | mpu.S);
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    mpu.P &= ~MPU::Flag::N & ~MPU::Flag::Z;
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0);
    mpu.T = 0;
    mpu.PC += 1;
}
void opPLP(MPU& mpu) {
    mpu.S++;
    mpu.P = readMem(mpu, 0x0100 | mpu.S) & ~MPU::Flag::B & ~MPU::Flag::EmptyBit;
    mpu.T = 0;
    mpu.PC += 1;
}
OpCode pullOperation(void (*handler)(MPU& mpu)) {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, handlerNop, handler, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}

// BRK instruction (called for hardware interrupt)
void brkPushPCH(MPU& mpu) {
    writeMem(mpu, 0x0100 | mpu.S, ((mpu.PC + (mpu.handlingIRQorNMI ? 0 : 2)) >> 8) & 0xFF);
    mpu.S--;
    mpu.T++;
}
void brkPushPCL(MPU& mpu) {
    writeMem(mpu, 0x0100 | mpu.S, (mpu.PC + (mpu.handlingIRQorNMI ? 0 : 2)) & 0xFF);
    mpu.S--;
    mpu.T++;
}
void brkPushP(MPU& mpu) {
    writeMem(mpu, 0x0100 | mpu.S, mpu.P | MPU::Flag::EmptyBit | (mpu.handlingIRQorNMI ? 0 : MPU::Flag::B));
    mpu.P |= MPU::Flag::I; // disable IRQ
    mpu.S--;
    mpu.T++;
}
void brkFetchAddrLow(MPU& mpu) {
    mpu.PC = readMem(mpu, mpu.handlingNMI ? 0xFFFA : 0xFFFE);
    mpu.T++;
}
void brkFetchAddrHigh(MPU& mpu) {
    mpu.PC |= readMem(mpu, mpu.handlingNMI ? 0xFFFB : 0xFFFF) << 8;
    mpu.handlingNMI = false;
    mpu.handlingIRQorNMI = false;
    mpu.T = 0;
}
OpCode createBRKOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, handlerNop, brkPushPCH, brkPushPCL, brkPushP, brkFetchAddrLow, brkFetchAddrHigh, undefinedOpcode };
    return opcodeData;
}

// Jump & Jump To Subroutine
void jsrPushPCH(MPU& mpu) {
    writeMem(mpu, 0x0100 | mpu.S, ((mpu.PC + 2) >> 8) & 0xFF);
    mpu.S--;
    mpu.T++;
}
void jsrPushPCL(MPU& mpu) {
    writeMem(mpu, 0x0100 | mpu.S, (mpu.PC + 2) & 0xFF);
    mpu.S--;
    mpu.T++;
}
void fetchJSRHighAddr(MPU& mpu) {
    mpu.effectiveAddr |= readMem(mpu, mpu.PC + 2) << 8;
    mpu.T = 0;
    mpu.PC = mpu.effectiveAddr;
}
OpCode createJSROpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteLowAddr, handlerNop, jsrPushPCH, jsrPushPCL, fetchJSRHighAddr, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
OpCode createJMPAbsolute() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchJSRHighAddr, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
void fetchJMPIndirAddrLow(MPU& mpu) {
    mpu.indirectAddr = readMem(mpu, mpu.PC + 1);
    mpu.T++;
}
void fetchJMPIndirAddrHigh(MPU& mpu) {
    mpu.indirectAddr |= (readMem(mpu, mpu.PC + 2) << 8);
    mpu.T++;
}
void fetchJMPEffAddrLow(MPU& mpu) {
    mpu.effectiveAddr = readMem(mpu, mpu.indirectAddr);
    mpu.T++;
}
void fetchJMPEffAddrHigh(MPU& mpu) {
    // 6502 bug: JMP indirect can't cross page boundary
    mpu.effectiveAddr |= readMem(mpu, (mpu.indirectAddr & 0xFF00) | ((mpu.indirectAddr + 1) & 0x00FF)) << 8;
    mpu.T = 0;
    mpu.PC = mpu.effectiveAddr;
}
OpCode createJMPIndirect() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, fetchJMPIndirAddrLow, fetchJMPIndirAddrHigh, fetchJMPEffAddrLow, fetchJMPEffAddrHigh, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}

// RTI instruction (return from interrupt)
void rtiPullP(MPU& mpu) {
    mpu.S++;
    mpu.P = readMem(mpu, 0x0100 | mpu.S) & ~MPU::Flag::B & ~MPU::Flag::EmptyBit;
    mpu.T++;
}
void rtiPullPCL(MPU& mpu) {
    mpu.S++;
    mpu.PC = readMem(mpu, 0x0100 | mpu.S);
    mpu.T++;
}
void rtiPullPCH(MPU& mpu) {
    mpu.S++;
    mpu.PC |= readMem(mpu, 0x0100 | mpu.S) << 8;
    mpu.T = 0;
}
OpCode createRTIOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, handlerNop, handlerNop, rtiPullP, rtiPullPCL, rtiPullPCH, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}
void rtsPullPCH(MPU& mpu) {
    mpu.S++;
    mpu.PC |= readMem(mpu, 0x0100 | mpu.S) << 8;
    mpu.T++;
}
void rtsLastCycle(MPU& mpu) {
    mpu.T = 0;
    mpu.PC += 1;
}
OpCode createRTSOpCode() {
    OpCode opcodeData;
    opcodeData.handlers = { fetchOpCode, handlerNop, handlerNop, rtiPullPCL, rtsPullPCH, rtsLastCycle, undefinedOpcode, undefinedOpcode };
    return opcodeData;
}

/* =============================================================================
 *                             ILLEGAL OPCODES
 * =============================================================================
 * Stable Opcodes:
 * SLO (ASO)                  <- ASL + ORA
 * RLA (RLN)                  <- ROL + AND
 * SRE (LSE)                  <- LSR + EOR
 * RRA (RRD)                  <- ROR + ADC
 * SAX (AXS,AAX)              <- STA + STX
 * LAX                        <- LDA + LDX
 * DCP (DCM)                  <- DEC + CMP
 * ISC (ISB,INS)              <- INC + SBC
 * ANC (ANC2,ANA,ANB)         <- AND + ASL/ROL
 * ALR (ASR)                  <- AND + LSR
 * ARR                        <- AND + ROR
 * SBX (AXS,SAX,XMA)          <- CMP + DEX
 * SBC (USBC,USB)             <- SBC + NOP
 *
 * Unstable Opcodes:
 * SHA (AXA,AHX,TEA)          <- Combinations of STA/STX/STY
 * SHX (A11,SXA,XAS,TEX)      <- Combinations of STA/STX/STY
 * SHY (A11,SYA,SAY,TEY)      <- Combinations of STA/STX/STY
 * TAS (XAS,SHS)              <- Combinations of STA/TXS + LDA/TSX
 * LAS (LAR)                  <- Combinations of STA/TXS + LDA/TSX
 *
 * Highly Unstable Opcodes:
 * ANE (XAA,AXM)              <- Combination of immediate and an implied command
 * LAX #imm (ATX,LXA,OAL,ANX) <- Combination of immediate and an implied command
 *
 * Source: https://csdb.dk/release/?id=198357
 */

void setEffectiveAbsYNoCarry(MPU& mpu) {
    mpu.effectiveAddr = mpu.effectiveAddr & 0xFF00 | (mpu.effectiveAddr + mpu.Y) & 0x00FF;
    readMem(mpu, mpu.effectiveAddr);
    mpu.T++;
}
void setEffectiveAbsY(MPU& mpu) {
    mpu.effectiveAddr = mpu.effectiveAddr + mpu.Y;
    mpu.modVal = readMem(mpu, mpu.effectiveAddr);
    mpu.T++;
}
void setEffectiveAbsXNoCarry(MPU& mpu) {
    mpu.effectiveAddr = mpu.effectiveAddr & 0xFF00 | (mpu.effectiveAddr + mpu.X) & 0x00FF;
    readMem(mpu, mpu.effectiveAddr);
    mpu.T++;
}
void setEffectiveAbsX(MPU& mpu) {
    mpu.effectiveAddr = mpu.effectiveAddr + mpu.X;
    mpu.modVal = readMem(mpu, mpu.effectiveAddr);
    mpu.T++;
}

void opDCPMod(MPU& mpu) {
    writeMem(mpu, mpu.effectiveAddr, mpu.modVal); // ghost write
    mpu.modVal--;
    opCMP(mpu, mpu.modVal);
    mpu.T++;
}
void opISCMod(MPU& mpu) {
    writeMem(mpu, mpu.effectiveAddr, mpu.modVal); // ghost write
    mpu.modVal++;
    opSBC(mpu, mpu.modVal);
    mpu.T++;
}
void opRRAMod(MPU& mpu) {
    opRORMod(mpu);
    mpu.T--; // reverse effect from op*Mod()
    opADC(mpu, mpu.modVal);
    mpu.T++;
}
void opRLAMod(MPU& mpu) {
    opROLMod(mpu);
    mpu.T--; // reverse effect from op*Mod()
    opAND(mpu, mpu.modVal);
    mpu.T++;
}
void opSLOMod(MPU& mpu) {
    opASLMod(mpu);
    mpu.T--; // reverse effect from op*Mod()
    opORA(mpu, mpu.modVal);
    mpu.T++;
}
void opSREMod(MPU& mpu) {
    opLSRMod(mpu);
    mpu.T--; // reverse effect from op*Mod()
    opEOR(mpu, mpu.modVal);
    mpu.T++;
}

OpCode createIllegalAbsYIndexed(void (*handler)(MPU&)) {
    return { .handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr,
                           setEffectiveAbsYNoCarry, setEffectiveAbsY, handler, storeModValLen3 } };
}
OpCode createIllegalAbsXIndexed(void (*handler)(MPU&)) {
    return { .handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr,
                           setEffectiveAbsXNoCarry, setEffectiveAbsX, handler, storeModValLen3 } };
}

void fetchIndirectGhostRead(MPU& mpu) {
    readMem(mpu, mpu.baseAddr);
    mpu.T++;
}
void fetchIllegalIndirectData(MPU& mpu) {
    mpu.modVal = readMem(mpu, mpu.effectiveAddr);
    mpu.T++;
}
void fetchIllegalZeropageIndirectYsetEffectiveAbsYNoCarry(MPU& mpu) {
    mpu.effectiveAddr = mpu.baseAddr & 0xFF00 | (mpu.baseAddr + mpu.Y) & 0x00FF;
    readMem(mpu, mpu.effectiveAddr);
    mpu.T++;
}
void fetchIllegalZeropageIndirectYsetEffectiveAbsY(MPU& mpu) {
    mpu.effectiveAddr = mpu.baseAddr + mpu.Y;
    mpu.modVal = readMem(mpu, mpu.effectiveAddr);
    mpu.T++;
}
OpCode createIllegalZeropageXIndexedIndirect(void (*handler)(MPU&)) {
    return { .handlers = { fetchOpCode, fetchIndirectXBase, fetchIndirectGhostRead, fetchIndirectXAddrLow, fetchIndirectXAddrHigh,
                           fetchIllegalIndirectData, handler, storeModValLen2 } };
}
OpCode createIllegalZeropageIndirectYIndexed(void (*handler)(MPU&)) {
    return { .handlers = { fetchOpCode, fetchIndirectYIndirectAddr, fetchIndirectYAddrLow, fetchIndirectYAddrHigh,
                           fetchIllegalZeropageIndirectYsetEffectiveAbsYNoCarry, fetchIllegalZeropageIndirectYsetEffectiveAbsY,
                           handler, storeModValLen2 } };
}

void opANC(MPU& mpu, uint8_t value) {
    opAND(mpu, value);
    bool carry = mpu.A & 0x80;
    mpu.P &= ~MPU::Flag::C;
    mpu.P |= carry ? MPU::Flag::C : 0;
}
void opALR(MPU& mpu, uint8_t value) {
    opAND(mpu, value);
    mpu.modVal = mpu.A;
    opLSRMod(mpu);
    mpu.T--; // reverse effect of op*Mod()
    mpu.A = mpu.modVal;
}
void opARR(MPU& mpu, uint8_t value) {
    opAND(mpu, value);
    mpu.modVal = mpu.A;
    uint8_t beforeRotate = mpu.modVal;
    opROR(mpu);
    mpu.T--; // reverse effect of op*Mod()

    bool carry = mpu.modVal & 0x40;
    bool overflow = (beforeRotate ^ mpu.modVal) & 0x40;
    mpu.P &= ~MPU::Flag::C & ~MPU::Flag::V;
    mpu.P |= (carry ? MPU::Flag::C : 0) | (overflow ? MPU::Flag::V : 0);
    mpu.A = mpu.modVal;
}
void opSBX(MPU& mpu, uint8_t value) {
    uint8_t oldVal = mpu.A & mpu.X;
    uint8_t sub = oldVal - value;
    mpu.X = sub;

    bool neg = static_cast<int8_t>(sub) < 0;
    bool zero = sub == 0;
    bool carry = static_cast<int16_t>(oldVal) - static_cast<int16_t>(value) - 0x01 >= 0;
    mpu.P &= ~MPU::Flag::N & ~MPU::Flag::Z & ~MPU::Flag::C;
    mpu.P |= (neg ? MPU::Flag::N : 0) | (zero ? MPU::Flag::Z : 0) | (carry ? MPU::Flag::C : 0);
}

void opLAX(MPU& mpu, uint8_t value) {
    opLDA(mpu, value);
    mpu.X = value;
}


std::array<OpCode, 256> createOpcodes() {
    std::array<OpCode, 256> opcodes{};
    for(auto& op : opcodes) op.handlers = { fetchOpCode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };

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
    // ILLEGAL INSTR: immediate
    opcodes[0x0B] = immediateMode(opANC);
    opcodes[0x2B] = immediateMode(opANC);
    opcodes[0x4B] = immediateMode(opALR);
    opcodes[0x6B] = immediateMode(opARR);
    opcodes[0xCB] = immediateMode(opSBX);
    opcodes[0xEB] = immediateMode(opSBC);
    // ILLEGAL INSTR: block 1
    opcodes[0x07] = zeroPageMode( [](MPU& mpu, uint8_t val) { mpu.modVal = val; opSLOMod(mpu); mpu.T--; });
    opcodes[0x17] = zeroPageXMode([](MPU& mpu, uint8_t val) { mpu.modVal = val; opSLOMod(mpu); mpu.T--; });
    opcodes[0x0F] = absoluteMode( [](MPU& mpu, uint8_t val) { mpu.modVal = val; opSLOMod(mpu); mpu.T--; });
    opcodes[0x27] = zeroPageMode( [](MPU& mpu, uint8_t val) { mpu.modVal = val; opRLAMod(mpu); mpu.T--; });
    opcodes[0x37] = zeroPageXMode([](MPU& mpu, uint8_t val) { mpu.modVal = val; opRLAMod(mpu); mpu.T--; });
    opcodes[0x2F] = absoluteMode( [](MPU& mpu, uint8_t val) { mpu.modVal = val; opRLAMod(mpu); mpu.T--; });
    opcodes[0x47] = zeroPageMode( [](MPU& mpu, uint8_t val) { mpu.modVal = val; opSREMod(mpu); mpu.T--; });
    opcodes[0x57] = zeroPageXMode([](MPU& mpu, uint8_t val) { mpu.modVal = val; opSREMod(mpu); mpu.T--; });
    opcodes[0x4F] = absoluteMode( [](MPU& mpu, uint8_t val) { mpu.modVal = val; opSREMod(mpu); mpu.T--; });
    opcodes[0x67] = zeroPageMode( [](MPU& mpu, uint8_t val) { mpu.modVal = val; opRRAMod(mpu); mpu.T--; });
    opcodes[0x77] = zeroPageXMode([](MPU& mpu, uint8_t val) { mpu.modVal = val; opRRAMod(mpu); mpu.T--; });
    opcodes[0x6F] = absoluteMode( [](MPU& mpu, uint8_t val) { mpu.modVal = val; opRRAMod(mpu); mpu.T--; });
    // ILLEGA INSTR: block 2
    auto storeAXLen2     = [](MPU& mpu) { writeMem(mpu, mpu.effectiveAddr,               mpu.A & mpu.X); mpu.T = 0; mpu.PC += 2; };
    auto storeAXLen3     = [](MPU& mpu) { writeMem(mpu, mpu.effectiveAddr,               mpu.A & mpu.X); mpu.T = 0; mpu.PC += 3; };
    auto storeAbsXAXLen2 = [](MPU& mpu) { writeMem(mpu, (mpu.baseAddr + mpu.Y) & 0x00FF, mpu.A & mpu.X); mpu.T = 0; mpu.PC += 2; };
    opcodes[0x87] = {.handlers = { fetchOpCode, fetchZeroPageAddr, storeAXLen2, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode }};
    opcodes[0x97] = {.handlers = { fetchOpCode, fetchZeroPageYBase, handlerNop, storeAbsXAXLen2, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode }};
    opcodes[0x83] = {.handlers = { fetchOpCode, fetchIndirectXBase, handlerNop, fetchIndirectXAddrLow, fetchIndirectXAddrHigh, storeAXLen2, undefinedOpcode, undefinedOpcode }};
    opcodes[0x8F] = {.handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr, storeAXLen3, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode }};
    opcodes[0xA7] = zeroPageMode(opLAX);
    opcodes[0xB7] = zeroPageYMode(opLAX);
    opcodes[0xA3] = indirectXMode(opLAX);
    opcodes[0xB3] = indirectYMode(opLAX);
    opcodes[0xAF] = absoluteMode(opLAX);
    opcodes[0xBF] = absoluteYMode(opLAX);
    // ILLEGA INSTR: block 3
    opcodes[0xC7] = zeroPageMode( [](MPU& mpu, uint8_t val) { mpu.modVal = val; opDCPMod(mpu); mpu.T--; });
    opcodes[0xD7] = zeroPageXMode([](MPU& mpu, uint8_t val) { mpu.modVal = val; opDCPMod(mpu); mpu.T--; });
    opcodes[0xCF] = absoluteMode( [](MPU& mpu, uint8_t val) { mpu.modVal = val; opDCPMod(mpu); mpu.T--; });
    opcodes[0xE7] = zeroPageMode( [](MPU& mpu, uint8_t val) { mpu.modVal = val; opISCMod(mpu); mpu.T--; });
    opcodes[0xF7] = zeroPageXMode([](MPU& mpu, uint8_t val) { mpu.modVal = val; opISCMod(mpu); mpu.T--; });
    opcodes[0xEF] = absoluteMode( [](MPU& mpu, uint8_t val) { mpu.modVal = val; opISCMod(mpu); mpu.T--; });

    // Single Byte Instructions
    opcodes[0x0A] = impliedSingleByte(opASL);
    opcodes[0x18] = impliedSingleByte(opCLC);
    opcodes[0xD8] = impliedSingleByte(opCLD);
    opcodes[0x58] = impliedSingleByte(opCLI);
    opcodes[0xB8] = impliedSingleByte(opCLV);
    opcodes[0xCA] = impliedSingleByte(opDEX);
    opcodes[0x88] = impliedSingleByte(opDEY);
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
    // ILLEGAL INSTR: NOPs
    opcodes[0x1A] = impliedSingleByte(opNOP);
    opcodes[0x3A] = impliedSingleByte(opNOP);
    opcodes[0x5A] = impliedSingleByte(opNOP);
    opcodes[0x7A] = impliedSingleByte(opNOP);
    opcodes[0xDA] = impliedSingleByte(opNOP);
    opcodes[0xFA] = impliedSingleByte(opNOP);
    opcodes[0x80] = immediateMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x82] = immediateMode([](MPU& mpu, uint8_t val) {});
    opcodes[0xC2] = immediateMode([](MPU& mpu, uint8_t val) {});
    opcodes[0xE2] = immediateMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x89] = immediateMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x04] = zeroPageMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x44] = zeroPageMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x64] = zeroPageMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x14] = zeroPageXMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x34] = zeroPageXMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x54] = zeroPageXMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x74] = zeroPageXMode([](MPU& mpu, uint8_t val) {});
    opcodes[0xD4] = zeroPageXMode([](MPU& mpu, uint8_t val) {});
    opcodes[0xF4] = zeroPageXMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x0C] = absoluteMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x1C] = absoluteXMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x3C] = absoluteXMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x5C] = absoluteXMode([](MPU& mpu, uint8_t val) {});
    opcodes[0x7C] = absoluteXMode([](MPU& mpu, uint8_t val) {});
    opcodes[0xDC] = absoluteXMode([](MPU& mpu, uint8_t val) {});
    opcodes[0xFC] = absoluteXMode([](MPU& mpu, uint8_t val) {});

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
    opcodes[0x4E] = bitShiftAbsolute(opLSRMod);
    opcodes[0x46] = bitShiftZeroPage(opLSRMod);
    opcodes[0x56] = bitShiftZeroPageX(opLSRMod);
    opcodes[0x5E] = bitShiftAbsoluteX(opLSRMod);
    opcodes[0x2E] = bitShiftAbsolute(opROLMod);
    opcodes[0x26] = bitShiftZeroPage(opROLMod);
    opcodes[0x36] = bitShiftZeroPageX(opROLMod);
    opcodes[0x3E] = bitShiftAbsoluteX(opROLMod);
    opcodes[0x6E] = bitShiftAbsolute(opRORMod);
    opcodes[0x66] = bitShiftZeroPage(opRORMod);
    opcodes[0x76] = bitShiftZeroPageX(opRORMod);
    opcodes[0x7E] = bitShiftAbsoluteX(opRORMod);

    // Branch Operations
    opcodes[0x90] = branchOps(opBCC);
    opcodes[0xB0] = branchOps(opBCS);
    opcodes[0xF0] = branchOps(opBEQ);
    opcodes[0x30] = branchOps(opBMI);
    opcodes[0xD0] = branchOps(opBNE);
    opcodes[0x10] = branchOps(opBPL);
    opcodes[0x50] = branchOps(opBVC);
    opcodes[0x70] = branchOps(opBVS);

    // Pull/Push Operations
    opcodes[0x48] = pushOperation(opPHA);
    opcodes[0x08] = pushOperation(opPHP);
    opcodes[0x68] = pullOperation(opPLA);
    opcodes[0x28] = pullOperation(opPLP);

    // Jump (From Subroutine) & Return from Subroutine
    opcodes[0x4C] = createJMPAbsolute();
    opcodes[0x6C] = createJMPIndirect();
    opcodes[0x20] = createJSROpCode();
    opcodes[0x60] = createRTSOpCode();

    // misc operations
    opcodes[0x00] = createBRKOpCode();
    opcodes[0x40] = createRTIOpCode();

    // ILLEGAL: NEW ADDR MODE: Absolute Y indexed RMW
    opcodes[0xDB] = createIllegalAbsYIndexed(opDCPMod);
    opcodes[0xFB] = createIllegalAbsYIndexed(opISCMod);
    opcodes[0x7B] = createIllegalAbsYIndexed(opRRAMod);
    opcodes[0x3B] = createIllegalAbsYIndexed(opRLAMod);
    opcodes[0x1B] = createIllegalAbsYIndexed(opSLOMod);
    opcodes[0x5B] = createIllegalAbsYIndexed(opSREMod);

    // ILLEGAL: NEW ADDR MODE: Absolute X indexed RMW
    opcodes[0xDF] = createIllegalAbsXIndexed(opDCPMod);
    opcodes[0xFF] = createIllegalAbsXIndexed(opISCMod);
    opcodes[0x7F] = createIllegalAbsXIndexed(opRRAMod);
    opcodes[0x3F] = createIllegalAbsXIndexed(opRLAMod);
    opcodes[0x1F] = createIllegalAbsXIndexed(opSLOMod);
    opcodes[0x5F] = createIllegalAbsXIndexed(opSREMod);

    // ILLEGAL: NEW ADDR MODE: Zeropage X indexed indirect RMW
    opcodes[0xC3] = createIllegalZeropageXIndexedIndirect(opDCPMod);
    opcodes[0xE3] = createIllegalZeropageXIndexedIndirect(opISCMod);
    opcodes[0x23] = createIllegalZeropageXIndexedIndirect(opRRAMod);
    opcodes[0x63] = createIllegalZeropageXIndexedIndirect(opRLAMod);
    opcodes[0x03] = createIllegalZeropageXIndexedIndirect(opSLOMod);
    opcodes[0x43] = createIllegalZeropageXIndexedIndirect(opSREMod);

    // ILLEGAL: NEW ADDR MODE: Zeropage indirect Y indexed RMW
    opcodes[0xD3] = createIllegalZeropageIndirectYIndexed(opDCPMod);
    opcodes[0xF3] = createIllegalZeropageIndirectYIndexed(opISCMod);
    opcodes[0x33] = createIllegalZeropageIndirectYIndexed(opRRAMod);
    opcodes[0x73] = createIllegalZeropageIndirectYIndexed(opRLAMod);
    opcodes[0x13] = createIllegalZeropageIndirectYIndexed(opSLOMod);
    opcodes[0x53] = createIllegalZeropageIndirectYIndexed(opSREMod);

    // ILLEGAL unstable
    opcodes[0x93] = {.handlers = { fetchOpCode, fetchIndirectYIndirectAddr, fetchIndirectYAddrLow, fetchIndirectYAddrHigh, setEffectiveAddrIndY, [](MPU& mpu) { writeMem(mpu, mpu.effectiveAddr, mpu.A & mpu.X & ((mpu.effectiveAddr >> 8) + 1)); mpu.T = 0; mpu.PC += 2; }, undefinedOpcode, undefinedOpcode }};
    opcodes[0x9F] = {.handlers = { fetchOpCode, fetchAbsoluteYAddrLow, fetchAbsoluteYAddrHigh, setEffectiveAddrAbsY, [](MPU& mpu) { writeMem(mpu, mpu.effectiveAddr, mpu.A & mpu.X & ((mpu.effectiveAddr >> 8) + 1)); mpu.T = 0; mpu.PC += 3; }, undefinedOpcode, undefinedOpcode, undefinedOpcode }};
    opcodes[0x9E] = {.handlers = { fetchOpCode, fetchAbsoluteYAddrLow, fetchAbsoluteYAddrHigh, setEffectiveAddrAbsY, [](MPU& mpu) { writeMem(mpu, mpu.effectiveAddr, mpu.X & ((mpu.effectiveAddr >> 8) + 1)); mpu.T = 0; mpu.PC += 3; }, undefinedOpcode, undefinedOpcode, undefinedOpcode }};
    opcodes[0x9C] = {.handlers = { fetchOpCode, fetchAbsoluteXAddrLow, fetchAbsoluteXAddrHigh, setEffectiveAddrAbsX, [](MPU& mpu) { writeMem(mpu, mpu.effectiveAddr, mpu.Y & ((mpu.effectiveAddr >> 8) + 1)); mpu.T = 0; mpu.PC += 3; }, undefinedOpcode, undefinedOpcode, undefinedOpcode }};
    opcodes[0x9B] = {.handlers = { fetchOpCode, fetchAbsoluteYAddrLow, fetchAbsoluteYAddrHigh, setEffectiveAddrAbsY, [](MPU& mpu) { writeMem(mpu, mpu.effectiveAddr, mpu.X & ((mpu.effectiveAddr >> 8) + 1)); mpu.T = 0; mpu.S = mpu.A & mpu.X; mpu.PC += 3; }, undefinedOpcode, undefinedOpcode, undefinedOpcode }};

    return opcodes;
}
const std::array<OpCode, 256> opcodes = createOpcodes();

void MPU::tick(bool IRQ, bool NMI) {
    if (!NMI) NMI_valid = true;

    // if (T == 1) std::cout << std::hex << PC << " " << (int)opcode << '(' << (int)mem->read(PC) << " " << (int)mem->read(PC + 1) << " " << (int)mem->read(PC + 2) << ")\n";

    if (T == 0) {
        if (NMI_valid && NMI)
            handlingNMI = true;

        if ((!(P & Flag::I) && IRQ) || handlingNMI) {
            if(handlingNMI) NMI_valid = false;
            handlingIRQorNMI = true;
            opcode = 0x00; // BRK
            T = 1; // skip over fetchOpcode
            return; // this cycle is done, next cycle will execute BRK with cycle = 1
        }
    }

    opcodes[opcode].handlers[T](*this);
}
