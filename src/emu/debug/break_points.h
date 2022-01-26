#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <algorithm>

class BreakPointException : std::exception {
public:
    BreakPointException() { }
    const char* what() const override { return "break point hit"; }
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

    std::vector<uint16_t> instructionBreakpoints;

    bool breakPointHit = false;
    bool enable = true;
};
