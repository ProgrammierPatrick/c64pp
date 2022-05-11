#include "c64.h"

const bool traceMPU = false;
const bool traceKernal = false;
const bool enableBreakPoints = true;

void C64::tick() {
    if constexpr (traceMPU) {
        mpuTrace.tick();
    }
    if constexpr (traceKernal) {
        kernalTrace.tick();
    }

    try {
        vic.tick();

        /*if (vic.cycleInLine == 1) {
            std::cout << "\n";
            if (vic.y == 0) {
                std::cout << "\n";
                std::cout << "\n";
                std::cout << "000000000111111111122222222223333333333444444444455555555556666\n";
                std::cout << "123456789012345678901234567890123456789012345678901234567890123\n";
            }
        }
        std::cout << (mpuStunned ? "_" : "-");*/

        if (!mpuStunned) {
            mpu.tick(cia.IRQ || vic.IRQ , cia.NMI || keyboard->queryRestore());
        }

        cia.tick();

        // mpu is stunned at first read
        // the condition (BA && !lastMemWritten) is not quite correct as the MPU is stunned before! the first read access.
        // This is not possible here, so we will just wait this one cycle after the MPU is allowed to continue
        if (!vic.BA && !mpu.lastMemWritten)
            mpuStunned = true;

        // mpu can only be stunned while BA is low
        // normally should be done before mpu.tick(), but checking here to wait one exta cycle after the MPU is allowed to continue
        if (vic.BA)
            mpuStunned = false;

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

    if constexpr (enableBreakPoints) {
        if (mpu.T == 0) {
            breakPoints.checkInstr(mpu.PC);
        }
    }
}
