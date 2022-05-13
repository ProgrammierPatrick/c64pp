#pragma once

#include "input/keyboard_state.h"

#include "../emu/c64.h"
#include "../emu/floppy/floppy_drive.h"

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

    void singleStepMPU();
    int stepInstruction();
    int stepLine();
    int stepFrame();

    std::unique_ptr<C64> c64;
    std::unique_ptr<FloppyDrive> floppyDrive;
    std::unique_ptr<KeyboardState> keyboard;
};
