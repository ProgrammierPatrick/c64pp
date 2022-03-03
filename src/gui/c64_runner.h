#pragma once

#include "../emu/c64.h"
#include "input/keyboard_state.h"

#include <memory>

/**
 * @brief contains the current C64 instance to be referenced by the GUI
 *
 * This is designed so that the application can remain single threaded. If the C64 is run in realistic speed,
 * the main frame would start a QTimer set to trigger an event at 50Hz, which then calls C64Runner::stepFrame()
 * from he GUI thread.
 *
 * Also contains KeyboardState for easy access from all GUI components
 */
class C64Runner {
public:
    C64Runner() {
        hardReset();
    }

    void hardReset();

    void singleStepMPU() {
        c64->tick();
    }
    int stepInstruction() {
        int numTicks = 0;
        do {
            c64->tick();
            numTicks++;
        } while (c64->mpu.T != 0);
        return numTicks;
    }
    int stepLine() {
        int numTicks = 0;
        do {
            c64->tick();
            numTicks++;
        } while(c64->vic.cycleInLine != 1);
        return numTicks;
    }
    int stepFrame() {
        int numTicks = 0;
        do {
            c64->tick();
            numTicks++;
        } while(c64->vic.y != 0 || c64->vic.cycleInLine != 1);
        return numTicks;
    }

    std::unique_ptr<C64> c64;
    std::unique_ptr<KeyboardState> keyboard;
};
