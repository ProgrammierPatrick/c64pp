#include "mpu_trace.h"

#include "../mpu.h"

#include "../../gui/text_utils.h"

#include <sstream>
#include <iostream>

MPUTrace::MPUTrace(MPU* mpu) : mpu(mpu) {
    lastLastPC = lastPC = mpu->PC;
    lastT = mpu->T;
}

void MPUTrace::tick() {
    std::stringstream ss;
    if (mpu->T == 0) {
        // DEC, INC, STA, STX, STY

        // INC, DEC Absolute
        if (mpu->opcode == 0xEE || mpu->opcode == 0xCE) {
            uint16_t addr = mpu->mem->read(lastLastPC + 1, true) | (mpu->mem->read(lastLastPC + 2, true) << 8);
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr, true)) << "\n";
        }
        // INC, DEC zeropage
        if (mpu->opcode == 0xE6 || mpu->opcode == 0xC6) {
            uint8_t addr = mpu->mem->read(lastLastPC + 1, true);
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr, true)) << "\n";
        }
        // INC, DEC zeropage, x
        if (mpu->opcode == 0xF6 || mpu->opcode == 0xD6) {
            ss << "MEMORY [????] = ?? (INC / DEC zeropage, x)\n";
        }
        // INC, DEC absolute x
        if (mpu->opcode == 0xFE || mpu->opcode == 0xDE) {
            ss << "MEMORY [????] = ?? (INC / DEC absolute x)\n";
        }
        // STA, STX, STY Absolute
        if (mpu->opcode == 0x8C || mpu->opcode == 0x8D || mpu->opcode == 0x8E) {
            uint16_t addr = mpu->mem->read(lastLastPC + 1, true) | (mpu->mem->read(lastLastPC + 2, true) << 8);
            ss << "lastLastPC: " << toHexStr(lastLastPC) << '\n';
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr, true)) << "\n";
        }
        // STA, STX, STY zeropage
        if (mpu->opcode == 0x84 || mpu->opcode == 0x85 || mpu->opcode == 0x86) {
            uint8_t addr = mpu->mem->read(lastLastPC + 1, true);
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr, true)) << "\n";
        }
        // STA (indexed) absolute X
        if (mpu->opcode == 0x9D) {
            uint16_t addr = (mpu->mem->read(lastLastPC + 1, true) | (mpu->mem->read(lastLastPC + 2, true) << 8)) + mpu->X;
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr, true)) << "\n";
        }
        // STA (indexed) absolute Y
        if (mpu->opcode == 0x99) {
            uint16_t addr = (mpu->mem->read(lastLastPC + 1, true) | (mpu->mem->read(lastLastPC + 2, true) << 8)) + mpu->Y;
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr, true)) << "\n";
        }
        // STA indirect Y
        if (mpu->opcode == 0x91) {
            auto indirectAddr = mpu->mem->read(lastLastPC + 1, true);
            uint16_t addr = (mpu->mem->read(indirectAddr, true) | (mpu->mem->read(indirectAddr + 1, true) << 8)) + mpu->Y;
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr, true)) << "    [" << toHexStr(indirectAddr) << "] = " << toHexStr(mpu->mem->read(indirectAddr, true)) << "\n";
        }
        // STA indirect X
        if (mpu->opcode == 0x81) {
            uint8_t baseAddr = mpu->mem->read(lastLastPC + 1, true);
            uint16_t addr = mpu->mem->read((baseAddr + mpu->X) & 0x00FF, true) | (mpu->mem->read((baseAddr + mpu->X + 1) & 0x00FF, true) << 8);
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr, true)) << "    [" << toHexStr(baseAddr) << "] = " << mpu->mem->read(baseAddr, true) << "\n";
        }
        // STA zeropage X, STY zeropage X
        if (mpu->opcode == 0x95 || mpu->opcode == 0x94) {
            uint16_t addr = (mpu->mem->read(lastLastPC + 1, true) + mpu->X) & 0x00FF;
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr, true)) << "\n";
        }
        // STX zeropage Y
        if (mpu->opcode == 0x96) {
            uint16_t addr = (mpu->mem->read(lastLastPC + 1, true) + mpu->Y) & 0x00FF;
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr, true)) << "\n";
        }

        ss << "Stack: " << "S:" << toHexStr(mpu->S) << "    " << toHexStr(mpu->mem->read(0x100 + mpu->S - 3, true)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S - 2, true)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S - 1, true));
        ss << " [" << toHexStr(mpu->mem->read(0x100 + mpu->S, true)) << "] " << toHexStr(mpu->mem->read(0x100 + mpu->S + 1, true)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S + 2, true)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S + 3, true)) << '\n';

        ss << "INSTR " << toHexStr(mpu->PC) << ": " << toHexStr(mpu->mem->read(mpu->PC, true)) << " " << toHexStr(mpu->mem->read(mpu->PC + 1, true)) << " " << toHexStr(mpu->mem->read(mpu->PC + 2, true));

    }

    ss << "  A:" << toHexStr(mpu->A);
    ss << " X:" << toHexStr(mpu->X);
    ss << " Y:" << toHexStr(mpu->Y);
    ss << " S:" << toHexStr(mpu->S);
    ss << " P:" << (mpu->P & 0x80 ? 'N' : '-') << (mpu->P & 0x40 ? 'V' : '-');
    ss << '-' << (mpu->P & 0x10 ? 'B' : '-') << (mpu->P & 0x08 ? 'D' : '-');
    ss << (mpu->P & 0x04 ? 'I' : '-') << (mpu->P & 0x02 ? 'Z' : '-') << (mpu->P & 0x01 ? 'C' : '-');
    ss << " PC:" << toHexStr(mpu->PC) << '\n';

    if(outputLines.size() >= 2000)
        outputLines.pop_front();
    outputLines.push_back(ss.str());

    lastT = mpu->T;

    if (mpu->T == 0) {
        lastLastPC = lastPC;
        lastPC = mpu->PC;
    }
}

void MPUTrace::printHistoryToConsole() {
    for(auto& str : outputLines)
        std::cout << str;
    std::cout << std::endl;
}

void MPUTrace::printStateToConsole() {
    std::stringstream ss;
    ss << "  A:" << toHexStr(mpu->A);
    ss << " X:" << toHexStr(mpu->X);
    ss << " Y:" << toHexStr(mpu->Y);
    ss << " S:" << toHexStr(mpu->S);
    ss << " P:" << (mpu->P & 0x80 ? 'N' : '-') << (mpu->P & 0x40 ? 'V' : '-');
    ss << '-' << (mpu->P & 0x10 ? 'B' : '-') << (mpu->P & 0x08 ? 'D' : '-');
    ss << (mpu->P & 0x04 ? 'I' : '-') << (mpu->P & 0x02 ? 'Z' : '-') << (mpu->P & 0x01 ? 'C' : '-');
    ss << " PC:" << toHexStr(mpu->PC) << '\n';

    ss << "Stack: " << "S:" << toHexStr(mpu->S) << "    " << toHexStr(mpu->mem->read(0x100 + mpu->S - 3, true)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S - 2, true)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S - 1, true));
    ss << " [" << toHexStr(mpu->mem->read(0x100 + mpu->S, true)) << "] " << toHexStr(mpu->mem->read(0x100 + mpu->S + 1, true)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S + 2, true)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S + 3, true)) << '\n';

    ss << "INSTR " << toHexStr(mpu->PC) << ": " << toHexStr(mpu->mem->read(mpu->PC, true)) << " " << toHexStr(mpu->mem->read(mpu->PC + 1, true)) << " " << toHexStr(mpu->mem->read(mpu->PC + 2, true));

    std::cout << ss.str() << std::flush;
}
