#include "via.h"

#include "../text_utils.h"

#include <iostream>

void VIA::tick() {
    if (!ca1IRQ || !paLatchingEnabled)
        paLatchValue = paIn & ~ddra | paLatchValue & ddra;
    paOut = paLatchValue;

    pbOut = pbIn & ~ddrb | pbOut & ddrb;

    // check for rising edge at CA1
    if (!lastCA1In && ca1In)
        ca1IRQ = true;
    lastCA1In = ca1In;

    // Timer 1 does not handle interrupts in one-shot mode correctly. In one-shot, the interrupt
    // is not fired the next time around. This should not matter for 1541, because T1 interrupts are not enabled in free running mode.
    // TODO: remove this check if 1541 is working without triggering this
    if (!t1FreeRunning && enableT1Interrupt)
        std::cout << "VIA: timer 1 running in one-shot mode while T1 interrupts are enabled. This is not supported." << std::endl;

    t1Counter--;
    if (t1Counter == 0x0000) {
        t1IRQ = true;
        if (t1FreeRunning)
            t1Counter = t1Latch;
    }
}

uint8_t VIA::read(uint8_t addr, bool nonDestructive) {
    // if (!nonDestructive && addr != 0) std::cout << "VIA: read " << toHexStr(addr) << std::endl;
    switch(addr) {
    case 0x0: // ORB
        return pbOut;
    case 0x1: // ORA
        if (!nonDestructive)
            ca1IRQ = false;
        return paLatchValue;
    case 0x2: // DDRB
        return ddrb;
    case 0x3: // DDRA
        return ddra;
    case 0x4: // TIC-L
        t1IRQ = false;
        return t1Counter & 0x00FF;
    case 0x5: // TIC-H
        return t1Counter >> 8;
    case 0x06: // T1L-L
        return t1Latch & 0x00FF;
    case 0x7: // T1L-H
        return t1Latch >> 8;
    case 0xB: // ACR (auxiliary control register)
        return (t1FreeRunning ? 0x40 : 0) | (paLatchingEnabled ? 0x01 : 0);
    case 0xC: // PCR (peripheral control register)
        return (cb2OutputHigh ? 0xE0 : 0) | (ca2OutputHigh ? 0x0E : 0);
    case 0xD: // IFR (interrupt flag register)
        return (getIRQ() ? 0x80 : 0) | (t1IRQ ? 0x40 : 0) | (ca1IRQ ? 0x02 : 0);
    case 0xE: // IER (interrupt enable register)
        return (enableT1Interrupt ? 0x40 : 0) | (enableCA1Interrupt ? 0x02 : 0);
    }
    if (!nonDestructive) std::cout << "VIA: undefined read at " << toHexStr(addr) << std::endl;
    return 0x00;
}

void VIA::write(uint16_t addr, uint8_t data) {
    // if (addr != 0) std::cout << "VIA: write [" << toHexStr(addr) << "] = " << toHexStr(data) << std::endl;
    switch(addr) {
    case 0x0: // ORB
        pbOut &= ~ddrb;
        pbOut |= ddrb & data;
        return;
    case 0x1: // ORA
        paLatchValue &= ~ddra;
        paLatchValue |= ddra & data;
        ca1IRQ = false;
        return;
    case 0x2: // DDRB
        ddrb = data;
        return;
    case 0x3: // DDRA
        ddra = data;
        return;
    case 0x4: // T1L-L
        t1Latch = t1Latch & 0xFF00 | data;
        return;
    case 0x5: // T1C-H
        t1Latch = t1Latch & 0x00FF | (data << 8);
        t1Counter = t1Latch;
        t1IRQ = false;
        return;
    case 0x6: // T1L-L
        t1Latch = t1Latch & 0xFF00 | data;
        return;
    case 0x7: // T1L-H
        t1Latch = t1Latch & 0x00FF | (data << 8);
        t1IRQ = false;
        return;
    case 0xB: // ACR (auxiliary control register)
        if (data & 0x3E) std::cout << "VIA: write to ACR sets unimplemented features." << std::endl;
        if ((data >> 6) != 0 && (data >> 6) != 1) std::cout << "VIA: write to ACR sets T1 to unknown mode. Supported are only: 00,01" << std::endl;
        t1FreeRunning = data >> 6;
        paLatchingEnabled = data & 0x01;
        return;
    case 0xC: // PCR (peripheral control register)
        if (((data >> 1) & 0x7) != 0 && ((data >> 1) & 0x7) != 0x2 && ((data >> 1) & 0x7) != 0x6 && ((data >> 1) & 0x7) != 0x7) std::cout << "VIA: write to PCR sets CA2 to unknown mode. Supported are only: 000,010,110,111. data:" << toHexStr(data) << std::endl;
        if (((data >> 5) & 0x7) != 0 && ((data >> 5) & 0x7) != 0x2 && ((data >> 5) & 0x7) != 0x6 && ((data >> 5) & 0x7) != 0x7) std::cout << "VIA: write to PCR sets CB2 to unknown mode. Supported are only: 000,010,110,111. data:" << toHexStr(data) << std::endl;
        ca2OutputHigh = ((data >> 1) & 0x7) == 0x7;
        cb2OutputHigh = ((data >> 5) & 0x7) == 0x7;
        return;
    case 0xD: // IFR (interrupt flag register)
        if (data & 0x3D) std::cout << "VIA: write to IFR contains unimplemented interrupt flags. Support are: 02, 40" << std::endl;
        if (data & 0x02) ca1IRQ = false;
        if (data & 0x40) t1IRQ = false;
        return;
    case 0xE: // IER (interrupt enable register)
        if (data & 0x80 && data & 0x3D)
            std::cout << "VIA: write to IRE contains unimplemented interrupt flags. Support are: 02, 40. data:" << toHexStr(data) << std::endl;
        bool set = data & 0x80;
        if (set && (data & 0x02))
            enableCA1Interrupt = true;
        if (set && (data & 0x40))
            enableT1Interrupt = true;
        if (!set && (data & 0x02))
            enableCA1Interrupt = false;
        if (!set && (data & 0x40))
            enableT1Interrupt = false;
        return;
    }
    std::cout << "VIA: undefied write! [" << toHexStr(addr) << "] = " << toHexStr(data) << std::endl;
}
