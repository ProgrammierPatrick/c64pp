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
    uint16_t lastPC;
    uint16_t lastLastPC;
    uint8_t lastT;

private:
    std::deque<std::string> outputLines;

    MPU* mpu;
};
