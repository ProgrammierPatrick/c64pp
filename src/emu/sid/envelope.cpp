#include "envelope.h"

#include <array>

const std::array<uint16_t,16> lfsr15CompareValLUT = {
    0x7F00, 0x0006, 0x003C, 0x0330,
    0x20C0, 0x6755, 0x3800, 0x500E,
    0x1212, 0x0222, 0x1848, 0x59B8,
    0x3840, 0x77E2, 0x7625, 0x0A93
};


void Envelope::setGate(bool gate) {
    if (this->gate != gate) {
        countingEnabled = true;
        countingUp = gate;
    }
    this->gate = gate;
}

uint8_t Envelope::tick() {
    // ----- tick 15 bit LFSR for main clock
    uint8_t lfsr15Rate = countingUp ? attack : (gate ? decay : release);
    bool resetLFSR15 = lfsr15 == lfsr15CompareValLUT[lfsr15Rate];

    if (resetLFSR15) lfsr15 = 0x7FFF;
    else lfsr15 = ((lfsr15 << 1) | (((lfsr15 >> 14) & 0x01) ^ ((lfsr15 >> 13) & 0x01))) & 0x7FFF;

    // ----- tick 5 bit LFSR for nonlinear decay / release
    if (counter == 0x00) {
        lfsr5Selection &= ~0x10;
    } else if (counter == 0x06) {
        lfsr5Selection &= ~0x08;
        lfsr5Selection |=  0x10;
    } else if (counter == 0x0E) {
        lfsr5Selection &= ~0x14;
        lfsr5Selection |=  0x08;
    } else if (counter == 0x1A) {
        lfsr5Selection &= ~0x0A;
        lfsr5Selection |=  0x04;
    } else if (counter == 0x36) {
        lfsr5Selection &= ~0x05;
        lfsr5Selection |=  0x02;
    } else if (counter == 0x5D) {
        lfsr5Selection &= ~0x02;
        lfsr5Selection |=  0x01;
    } else if (counter == 0xFF) {
        lfsr5Selection &= ~0x01;
    }

    bool fixedPointFound = lfsr5Selection == 0;

    bool resetLFSR5 = resetLFSR15 && (
               countingUp
            || fixedPointFound
            || (lfsr5Selection & 0x01) && lfsr5 == 0x1C
            || (lfsr5Selection & 0x02) && lfsr5 == 0x11
            || (lfsr5Selection & 0x04) && lfsr5 == 0x1B
            || (lfsr5Selection & 0x08) && lfsr5 == 0x08
            || (lfsr5Selection & 0x10) && lfsr5 == 0x0F);

    if (resetLFSR15) {
        if (resetLFSR5) lfsr5 = 0x1F;
        else lfsr5 = ((lfsr5 << 1) | (((lfsr5 >> 4) & 0x01) ^ ((lfsr5 >> 2) & 0x01))) & 0x1F;
    }

    // ----- update counter
    if (countingEnabled && resetLFSR5) {
        if (countingUp) counter++;
        else            counter--;
    }

    // ----- update counter state
    if (counter == 0xFF)
        countingUp = false;
    if (gate && !countingUp && counter == (sustain | (sustain << 4)))
        countingEnabled = false;
    if (gate && !countingUp && counter < (sustain | (sustain << 4)))
        countingEnabled = true;
    if (!gate && counter == 0x00)
        countingEnabled = false;

    return counter;
}
