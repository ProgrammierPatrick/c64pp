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
            uint16_t addr = mpu->mem->read(lastLastPC + 1) | (mpu->mem->read(lastLastPC + 2) << 8);
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr)) << "\n";
        }
        // INC, DEC zeropage
        if (mpu->opcode == 0xE6 || mpu->opcode == 0xC6) {
            uint8_t addr = mpu->mem->read(lastLastPC + 1);
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr)) << "\n";
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
            uint16_t addr = mpu->mem->read(lastLastPC + 1) | (mpu->mem->read(lastLastPC + 2) << 8);
            ss << "lastLastPC: " << toHexStr(lastLastPC) << '\n';
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr)) << "\n";
        }
        // STA, STX, STY zeropage
        if (mpu->opcode == 0x84 || mpu->opcode == 0x85 || mpu->opcode == 0x86) {
            uint8_t addr = mpu->mem->read(lastLastPC + 1);
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr)) << "\n";
        }
        // STA (indexed) absolute X
        if (mpu->opcode == 0x9D) {
            uint16_t addr = (mpu->mem->read(lastLastPC + 1) | (mpu->mem->read(lastLastPC + 2) << 8)) + mpu->X;
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr)) << "\n";
        }
        // STA (indexed) absolute Y
        if (mpu->opcode == 0x99) {
            uint16_t addr = (mpu->mem->read(lastLastPC + 1) | (mpu->mem->read(lastLastPC + 2) << 8)) + mpu->Y;
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr)) << "\n";
        }
        // STA indirect Y
        if (mpu->opcode == 0x91) {
            auto indirectAddr = mpu->mem->read(lastLastPC + 1);
            uint16_t addr = (mpu->mem->read(indirectAddr) | (mpu->mem->read(indirectAddr + 1) << 8)) + mpu->Y;
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr)) << "    [" << toHexStr(indirectAddr) << "] = " << toHexStr(mpu->mem->read(indirectAddr)) << "\n";
        }
        // STA indirect X
        if (mpu->opcode == 0x81) {
            uint8_t baseAddr = mpu->mem->read(lastLastPC + 1);
            uint16_t addr = mpu->mem->read((baseAddr + mpu->X) & 0x00FF) | (mpu->mem->read((baseAddr + mpu->X + 1) & 0x00FF) << 8);
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr)) << "    [" << toHexStr(baseAddr) << "] = " << mpu->mem->read(baseAddr) << "\n";
        }
        // STA zeropage X, STY zeropage X
        if (mpu->opcode == 0x95 || mpu->opcode == 0x94) {
            uint16_t addr = (mpu->mem->read(lastLastPC + 1) + mpu->X) & 0x00FF;
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr)) << "\n";
        }
        // STX zeropage Y
        if (mpu->opcode == 0x96) {
            uint16_t addr = (mpu->mem->read(lastLastPC + 1) + mpu->Y) & 0x00FF;
            ss << "MEMORY [" << toHexStr(addr) << "] = " << toHexStr(mpu->mem->read(addr)) << "\n";
        }

        ss << "Stack: " << "S:" << toHexStr(mpu->S) << "    " << toHexStr(mpu->mem->read(0x100 + mpu->S - 3)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S - 2)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S - 1));
        ss << " [" << toHexStr(mpu->mem->read(0x100 + mpu->S)) << "] " << toHexStr(mpu->mem->read(0x100 + mpu->S + 1)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S + 2)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S + 3)) << '\n';

        ss << "INSTR " << toHexStr(mpu->PC) << ": " << toHexStr(mpu->mem->read(mpu->PC)) << " " << toHexStr(mpu->mem->read(mpu->PC + 1)) << " " << toHexStr(mpu->mem->read(mpu->PC + 2));

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

    ss << "Stack: " << "S:" << toHexStr(mpu->S) << "    " << toHexStr(mpu->mem->read(0x100 + mpu->S - 3)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S - 2)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S - 1));
    ss << " [" << toHexStr(mpu->mem->read(0x100 + mpu->S)) << "] " << toHexStr(mpu->mem->read(0x100 + mpu->S + 1)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S + 2)) << " " << toHexStr(mpu->mem->read(0x100 + mpu->S + 3)) << '\n';

    ss << "INSTR " << toHexStr(mpu->PC) << ": " << toHexStr(mpu->mem->read(mpu->PC)) << " " << toHexStr(mpu->mem->read(mpu->PC + 1)) << " " << toHexStr(mpu->mem->read(mpu->PC + 2));

    std::cout << ss.str() << std::flush;
}
