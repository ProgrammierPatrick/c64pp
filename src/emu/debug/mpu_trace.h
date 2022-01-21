#pragma once

#include <deque>
#include <string>

class MPU;

class MPUTrace {
public:
    MPUTrace(MPU* mpu);

    void tick();
    void printHistoryToConsole();
    void printStateToConsole();

public:
    uint16_t lastPC = 0;
    uint16_t lastLastPC = 0;
    uint8_t lastT = 0;

private:
    std::deque<std::string> outputLines;

    MPU* mpu;
};
