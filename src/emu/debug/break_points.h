#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>

class BreakPointException : std::exception {
public:
    BreakPointException() : std::exception("break point hit") { }
};

class BreakPoints {
public:
    void checkInstr(uint16_t instr) {
        if (enable) {
            bool hit = std::find(instructionBreakpoints.begin(), instructionBreakpoints.end(), instr) != instructionBreakpoints.end();
            if (hit) {
                breakPointHit = true;
                throw BreakPointException{};
            }
        }
    }

    void resetBreakpoints() { breakPointHit = false; }

    bool breakPointHit = false;
    std::vector<uint16_t> instructionBreakpoints;
    bool enable = true;
};
