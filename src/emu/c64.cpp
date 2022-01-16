#include "c64.h"

const bool traceMPU = true;

void C64::tick() {
    if constexpr (traceMPU) {
        mpuTrace.tick();
    }

    try {
        vic.tick();
        mpu.tick(cia.IRQ, cia.NMI);
        cia.tick();
    }
    catch (std::runtime_error& e) {
        if constexpr (traceMPU) {
            mpuTrace.printHistoryToConsole();
        } else {
            mpuTrace.printStateToConsole();
        }
        mpuTrace.printHistoryToConsole();
        std::cout << "exception encountered: " << e.what() << std::endl;
        throw;
    }
}
