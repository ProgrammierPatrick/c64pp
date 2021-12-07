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
    std::array<void (*)(MPU&), 6> handlers;
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
    opcodeData.handlers = { fetchOpCode, fetchImmediate, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
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
    opcodeData.handlers = { fetchOpCode, fetchAbsoluteLowAddr, fetchAbsoluteHighAddr, fetchAbsoluteData, undefinedOpcode, undefinedOpcode };
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
    opcodeData.handlers = { fetchOpCode, fetchZeroPageAddr, fetchZeroPageData, undefinedOpcode, undefinedOpcode, undefinedOpcode };
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
    opcodeData.handlers = { fetchOpCode, fetchIndirectXBase, handlerNop, fetchIndirectXAddrLow, fetchIndirectXAddrHigh, fetchIndirectXData };
    opcodeData.dataHandler = handler;
    return opcodeData;
}

void opADC(MPU& mpu, uint8_t value) {
    uint8_t oldA = mpu.A;
    mpu.A = oldA + value + (mpu.P & MPU::Flag::C);
    bool neg = static_cast<int8_t>(mpu.A) < 0;
    bool zero = mpu.A == 0;
    bool carry = (oldA + static_cast<uint16_t>(value) + (mpu.P & MPU::Flag::C)) > 0xFFFF;
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

std::array<OpCode, 256> createOpcodes() {
    std::array<OpCode, 256> opcodes{};
    for(auto& op : opcodes) op.handlers = { fetchOpCode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode, undefinedOpcode };
    opcodes[0x69] = immediateMode(opADC);
    opcodes[0x6D] = absoluteMode(opADC);
    opcodes[0x65] = zeroPageMode(opADC);
    opcodes[0x61] = indirectXMode(opADC);
    opcodes[0x29] = immediateMode(opAND);
    opcodes[0x2D] = absoluteMode(opAND);
    opcodes[0x25] = zeroPageMode(opAND);
    opcodes[0x21] = indirectXMode(opAND);

    return opcodes;
}
std::array<OpCode, 256> opcodes = createOpcodes();

void MPU::tick() {
    opcodes[opcode].handlers[cycle](*this);
}
