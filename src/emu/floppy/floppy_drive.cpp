#include "floppy_drive.h"

#include "../text_utils.h"
#include <iostream>

const bool traceMPU = true;

void FloppyDrive::tick() {

    if constexpr (traceMPU) {
        mpuTrace.tick();
    }

    static uint64_t tickCount = 0;
    //if ((tickCount % 100000) == 0)
    //    std::cout << "Floppy Tick PC:" << toHexStr(mpu.PC) << std::endl;
    tickCount++;

    static bool lastATN = false, lastCLK = false, lastDATA = false;
    bool ATN = !serialBus->getAttention();
    bool CLK = !serialBus->getClock();
    bool DATA = !serialBus->getData();
    if (ATN != lastATN || CLK != lastCLK || DATA != lastDATA) {
        std::cout << "serial: ";
        if (ATN) {
            std::cout << "ATN[";
            for (auto d : serialBus->devices)
                if (d->pullAttention) std::cout << d->serialDeviceName;
            std::cout << "] ";
        }
        if (CLK) {
            std::cout << "CLK[";
            for (auto d : serialBus->devices)
                if (d->pullClock) std::cout << d->serialDeviceName;
            std::cout << "] ";
        }
        if (DATA) {
            std::cout << "DATA[";
            for (auto d : serialBus->devices)
                if (d->pullData) std::cout << d->serialDeviceName;
            std::cout << "] ";
        }
        std::cout << std::endl;
    }
    lastATN = ATN; lastCLK = CLK; lastDATA = DATA;



    // DISK STUFF
    // disk speed: 300 RPM (5 rps), 48 tracks per inch, 40 tracks
    // 3.25us/bit (track 1), 21 sectors (track 1)

    // 5s^-1 * 3.25us = 5r * 0.00000325 = 0.00001625r/bit => ~61538.46 bits per track
    // 3.25us/bit * 8 = 26us/byte
    static int bytePos = 0;
    static int sectorPos = 0;
    if ((tickCount % 26) == 0) {
        bytePos = (bytePos + 1) % 295;
        if (bytePos == 0) sectorPos = (sectorPos + 1) % 2;
    }


    bool writeProtect = true;

    // 0 = Data bytes are being currently read from disk; 1 = SYNC marks are being read.
    bool sync = bytePos < 5 || bytePos >= 25 && bytePos < 30;
    uint8_t data = 0xFF;
    const std::array<uint8_t, 0x100> bamData = {
        /*00*/ 0x12, 0x01, 0x41, 0x00, 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f,
        /*10*/ 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f,
        /*20*/ 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f,
        /*30*/ 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f,
        /*40*/ 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f,
        /*50*/ 0x13, 0xff, 0xff, 0x07, 0x13, 0xff, 0xff, 0x07, 0x13, 0xff, 0xff, 0x07, 0x13, 0xff, 0xff, 0x07,
        /*60*/ 0x13, 0xff, 0xff, 0x07, 0x12, 0xff, 0xff, 0x03, 0x12, 0xff, 0xff, 0x03, 0x12, 0xff, 0xff, 0x03,
        /*70*/ 0x12, 0xff, 0xff, 0x03, 0x12, 0xff, 0xff, 0x03, 0x12, 0xff, 0xff, 0x03, 0x11, 0xff, 0xff, 0x01,
        /*80*/ 0x11, 0xff, 0xff, 0x01, 0x11, 0xff, 0xff, 0x01, 0x11, 0xff, 0xff, 0x01, 0x11, 0xff, 0xff, 0x01,
        /*90*/ 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,
        /*A0*/ 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x32, 0x41, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00,
        /*B0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /*C0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /*D0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /*E0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /*F0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    const std::array<uint8_t, 0x100> sector1Data = {
        0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    auto& sectorData = (sectorPos == 0) ? bamData : sector1Data;
    // https://en.wikipedia.org/wiki/Group_coded_recording#Commodore
    const std::array<uint8_t, 16> gcr = { 0x0A, 0x0B, 0x12, 0x13, 0x0E, 0x0F, 0x16, 0x17, 0x09, 0x19, 0x1A, 0x1B, 0x0D, 0x1D, 0x1E, 0x15 };

    auto encodeGCR = [&gcr](auto pos, auto data) -> uint8_t {
        auto block = pos / 5;
        auto p = pos % 5;
        if (p == 0) {
            auto v1 = data[block * 4 + p];
            return (gcr[v1 >> 4] << 3) | (gcr[v1 & 0x0F] >> 2);
        } else if (p == 4) {
            auto v0 = data[block * 4 + p - 1];
            return (gcr[v0 >> 4] << 5) | gcr[v0 & 0x0F];
        } else {
            auto v0 = data[block * 4 + p - 1];
            auto v1 = data[block * 4 + p];
            if (p == 1)      return                       (gcr[v0 & 0x0F] << 6) | (gcr[v1 >> 4] << 1) | (gcr[v1 & 0x0F] >> 4);
            else if (p == 2) return                       (gcr[v0 & 0x0F] << 4) | (gcr[v1 >> 4] >> 1);
            else             return (gcr[v0 >> 4] << 7) | (gcr[v0 & 0x0F] << 2) | (gcr[v1 >> 4] >> 3);
        }
    };
    uint8_t trackNum = getTrack();
    uint8_t checksum = trackNum ^ sectorPos ^ 0x0A ^ 0x0A;
    std::array<uint8_t,8> headerData { 0x08, trackNum, (uint8_t)sectorPos, 0x0A, 0x0A, checksum, 0x0F, 0x0F };

    if (bytePos >= 5 && bytePos < 15) {
        // header
        data = encodeGCR(bytePos - 5, headerData);
        //std::cout << "header[" << bytePos - 5 << "]: " << toHexStr(data) << std::endl;
    }
    if (bytePos >= 30 && bytePos < 290) {
        // sector data
        data = encodeGCR(bytePos - 30, sectorData);
        //std::cout << "sector[" << bytePos - 30 << "]" << std::endl;
    }
    //if (sync) std::cout << "sync" << std::endl;


    if (tickCount == 1) std::cout << "GCR at 5 (start of header): 0x" << toHexStr(encodeGCR(0, headerData)) << std::endl;


    //static uint8_t lastStepValue = 0xFF; // 2-bit stepper moter value. increasing: move head "upwards", decreasing: move head "downwards"
    static uint8_t lastDensity = 0xFF; // 2-bit value that clocks down machanism for disk density adaption
    static bool lastMode = false;
    static bool lastMotor = true;
    static bool lastLED = true;
    static bool lastSOE = true;

    static bool lastIRQ = false;
    bool IRQ = via1.getIRQ() || via2.getIRQ();
    // if (IRQ != lastIRQ) std::cout << "floppy IRQ: " << (IRQ ? "ON" : "OFF") << std::endl;
    lastIRQ = IRQ;

    // bool byteReady = !((tickCount % 26) == 0 && lastSOE) && !sync;
    bool byteReady = ((tickCount % 26) == 0 || !lastSOE) && !sync;

    if (byteReady) mpu.P |= MPU::Flag::V;
    mpu.tick(via1.getIRQ() || via2.getIRQ());

    via1.pbIn = (!serialBus->getAttention() ? 0x80 : 0) | (!serialBus->getClock() ? 0x04 : 0) | (!serialBus->getData() ? 0x01 : 0);
    via1.ca1In = !serialBus->getAttention();
    via2.pbIn = (!sync ? 0x80 : 0) | (!writeProtect ? 0x10 : 0);
    via2.paIn = data;
    via2.ca1In = byteReady;

    via1.tick();
    via2.tick();

    static bool lastATNA = false;
    bool ATNA = via1.pbOut & 0x10;
    if (lastATNA != ATNA) std::cout << "ATNA: " << (ATNA ? "ON" : "OFF") << std::endl;
    lastATNA = ATNA;

    pullData = (via1.pbOut & 0x02) || (!serialBus->getAttention() != static_cast<bool>(via1.pbOut & 0x10));
    pullClock = via1.pbOut & 0x08;

    uint8_t step = via2.pbOut & 0x03;
    bool motor = via2.pbOut & 0x04;
    bool led = via2.pbOut & 0x08;
    uint8_t density = (via2.pbOut >> 5) & 0x03;
    bool SOE = via2.ca2OutputHigh;
    bool mode = via2.cb2OutputHigh;

    if (lastMotor != motor) std::cout << "1541 Motor: " << (motor ? "ON" : "OFF") << std::endl;
    if (lastLED != led) {
        std::cout << "1541 LED: " << (led ? "ON" : "OFF") << std::endl;
    }
    if (lastDensity != density) std::cout << "1541 density: " << toHexStr(density) << std::endl;
    // if (lastSOE != SOE) std::cout << "1541 SOE: " << (SOE ? "ON" : "OFF") << std::endl;
    if (lastMode != mode) std::cout << "1541 mode: " << (mode ? "READ" : "WRITE") << std::endl;

    if (step == (stepperMotorState + 1) % 4) headPosition++;
    if (step == (stepperMotorState + 3) % 4) headPosition = headPosition > 0 ? headPosition - 1 : 0;
    if (stepperMotorState != step) std::cout << "1541 stepper motor: " << toHexStr(step) << " head pos: " << headPosition << " track:" << (int)getTrack() << " PC:" << toHexStr(mpu.PC) << std::endl;
    stepperMotorState = step;

    lastMotor = motor;
    lastLED = led;
    lastDensity = density;
    lastSOE = SOE;
    lastMode = mode;

    if (mpu.T == 0) {
        static bool handleATN = false;
        if (mpu.PC == 0xFE67) {
        //    std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "SYSIRQ" << std::endl;
        } if (mpu.PC == 0xE853) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "ATNIRQ" << std::endl;
            handleATN = true;
        } if (mpu.PC == 0xE60A) {
            if constexpr (traceMPU) mpuTrace.printHistoryToConsole();
            for(int y = 0; y < 0x80; y++) {
                std::cout << toHexStr(static_cast<uint16_t>(y * 16)) << "| ";
                for (int x = 0; x < 16; x++)
                    std::cout << toHexStr(mpu.mem->read(y * 16 + x, true)) << ' ';
                for (int x = 0; x < 16; x++)
                    std::cout << fromPETSCII(mpu.mem->read(y * 16 + x, true));
                std::cout << std::endl;
            }
        } if (mpu.PC == 0xE85B) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "SRVATN" << std::endl;
        } if (mpu.PC == 0xE870) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "finished CLKHI, proceeding with DATLOW" << std::endl;
        } if (mpu.PC == 0xE99C) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "DATHI" << std::endl;
        } if (mpu.PC == 0xF2B0) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "LCC" << std::endl;
            handleATN = false;
        } if (mpu.PC == 0xF3B1) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "SEAK" << std::endl;
        } if (mpu.PC == 0xF3BB) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "SEAK: call SYNC" << std::endl;
        } if (mpu.PC == 0xF3C1) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "SEAK: read first byte at " << bytePos << std::endl;
        } if (mpu.PC == 0xF407) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "SEAK: not a header block" << std::endl;
        } if (mpu.PC == 0xF497) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "CNVRTN: convert GCR header to binary: ";
            for (int i = 0; i < 8; i++) std::cout << toHexStr(mpu.mem->read(0x24 + i, true)) << " ";
            std::cout << std::endl;
        } if (mpu.PC == 0xF3CE) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "SEAK: read header byte: " << toHexStr(mpu.A) << " at " << bytePos << std::endl;
        } if (mpu.PC == 0xF556) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "SYNC bytePos:" << bytePos << std::endl;
        } if (mpu.PC == 0xF56D) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "SYNC: found bytePos:" << bytePos << std::endl;
        } if (mpu.PC == 0xF7E6) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "GET4GB: decode GCR from: ";
            uint16_t bufpnt = (mpu.mem->read(0x30, true) | mpu.mem->read(0x31, true) << 8) + mpu.mem->read(0x34, true);
            for (int i = 0; i < 5; i++) std::cout << toHexStr(mpu.mem->read(bufpnt + i, true)) << " ";
            std::cout << std::endl;
        } if (mpu.PC == 0xF89F) {
            std::cout << "to: ";
            for (int i = 0; i < 4; i++) std::cout << toHexStr(mpu.mem->read(0x52 + i, true)) << " ";
            std::cout << std::endl;
        } if (mpu.PC == 0xEA71) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "PERR: " << toHexStr(mpu.mem->read(0x6F, true)) << std::endl;
        } if (mpu.PC == 0xC12C) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "ERRON" << std::endl;
        } if (mpu.mem->read(mpu.PC, true) == 0x40) {
            //std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "RTI" << std::endl;
        } if (mpu.mem->read(mpu.PC, true) == 0x40 && handleATN) {
            std::cout << "1541 [" << tickCount / 1'000'000.f << "] " << "RTI" << std::endl;
        }

        static uint8_t lastErword = 0;
        uint8_t erword = mpu.mem->read(0x026c, true);
        if (erword != lastErword) {
            // if constexpr (traceMPU) mpuTrace.printHistoryToConsole();
            std::cout << "ERWORD: " << toHexStr(erword) << std::endl;
            for (uint16_t addr = 0x02D5; addr <= 0x02F8; addr++)
                std::cout << fromPETSCII(mpu.mem->read(addr, true));
            std::cout << std::endl;
        }
        lastErword = erword;
    }

    //static uint64_t tickCount = 0;
    //if ((tickCount % 100000) == 0)
    //    std::cout << "VIA1 PB:" << toHexStr(via1.pbOut) << " VIA2 PB:" << toHexStr(via2.pbOut) << " PA:" << toHexStr(via1.paOut) << std::endl;
    //tickCount++;
}

uint8_t FloppyDriveMemoryView::read(uint16_t addr, bool nonDestructive) {
    if (addr < 0x0800) return ram->read(addr, nonDestructive);
    else if (addr >= 0x1800 && addr < 0x1810) return via1->read(addr - 0x1800, nonDestructive);
    else if (addr >= 0x1C00 && addr < 0x1C10) return via2->read(addr - 0x1C00, nonDestructive);
    else if (addr >= 0xC000) return dos->read(addr - 0xC000, nonDestructive);
    return 0x00;
}

void FloppyDriveMemoryView::write(uint16_t addr, uint8_t data) {
    if (addr < 0x800) ram->write(addr, data);
    else if (addr >= 0x1800 && addr < 0x1810) via1->write(addr - 0x1800, data);
    else if (addr >= 0x1C00 && addr < 0x1C10) via2->write(addr - 0x1C00, data);
    else if (addr >= 0xC000) dos->write(addr - 0xC000, data);
}
