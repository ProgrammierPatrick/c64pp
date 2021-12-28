#pragma once

#include <cstdint>
#include <iostream>

/*
 * CNT always high
 */
class Timer {
public:

    enum Inmode {
        INMODE_PHI2 = 0,
        INMODE_CNT = 1, // CNT is not connected
        INMODE_TA_UNDERFLOW = 2,
        INMODE_UNDERFLOW_CNT = 3 // effectively same as 2 as CNT is high
    };

    void tick(bool& underflowA, bool& underflowB) {
        underflowA = false;
        underflowB = false;
        if (startedA && inmodeA == INMODE_PHI2) {
            counterA--;
            if (counterA == 0xFFFF) {
                counterA = latchA;
                underflowA = true;
            }
        }
        if (startedB && (inmodeB == INMODE_PHI2 || (inmodeB == INMODE_TA_UNDERFLOW || inmodeB == INMODE_UNDERFLOW_CNT) && underflowA)) {
            counterB--;
            if (counterB == 0xFFFF) {
                counterB = latchB;
                underflowB = true;
            }
        }

    }

    void setTALO(uint8_t value) {
        latchA &= 0xFF00;
        latchA |= value;
    }

    void setTBLO(uint8_t value) {
        latchB &= 0xFF00;
        latchB |= value;
    }

    void setTAHI(uint8_t value) {
        latchA &= 0x00FF;
        latchA |= value << 8;
        if (!startedA) counterA = latchA;
    }

    void setTBHI(uint8_t value) {
        latchB &= 0x00FF;
        latchB |= value << 8;
        if (!startedB) counterB = latchB;
    }

    void writeCRA(uint8_t val) {
        startedA = val & 0x01;
        if (val & 0x02) {
            std::cout << "Warning: CIA PBON = 1 ignored" << std::endl;
        }
        oneshotA = val & 0x08;
        if (val & 0x10) counterA = latchA;
        inmodeA = static_cast<Inmode>((val >> 5) & 0x01);
    }

    void writeCRB(uint8_t val) {
        startedB = val & 0x01;
        if (val & 0x02) {
            std::cout << "Warning: CIA PBON = 1 ignored" << std::endl;
        }
        oneshotB = val & 0x08;
        if (val & 0x10) counterB = latchB;
        inmodeB = static_cast<Inmode>((val >> 5) & 0x03);
    }

    uint8_t readCRA() {
        uint8_t ret = 0x00;
        if (startedA) ret |= 0x01;
        if (oneshotA) ret |= 0x08;
        ret |= inmodeA << 5;
        return ret;
    }

    uint8_t readCRB() {
        uint8_t ret = 0x00;
        if (startedB) ret |= 0x01;
        if (oneshotB) ret |= 0x08;
        ret |= inmodeB << 5;
        return ret;
    }

    uint16_t counterA = 0;
    uint16_t latchA = 0;
    bool oneshotA = true;
    bool startedA = false;
    uint16_t counterB = 0;
    uint16_t latchB = 0;
    bool oneshotB = true;
    bool startedB = false;
    Inmode inmodeA = INMODE_PHI2;
    Inmode inmodeB = INMODE_PHI2;


};
