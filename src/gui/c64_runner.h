#pragma once

#include "../emu/c64.h"

#include <memory>

/**
 * @brief contains the current C64 instance to be referenced by the GUI
 *
 * This is designed so that the application can remain single threaded. If the C64 is run in realistic speed,
 * the main frame would start a QTimer set to trigger an event at 50Hz, which then calls C64Runner::stepFrame()
 * from he GUI thread.
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
    void stepFrame() {
        // 0,9852486 MHz / 50Hz ~ 19704 Ticks per frame
        // of couse not quite correct, since VIC controlls the MPU clock
        for(int i = 0; i < 19704; i++) {
            c64->tick();
        }
    }

    std::unique_ptr<C64> c64;
};
