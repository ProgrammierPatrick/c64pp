#pragma once

#include "via.h"

#include "../io/serial.h"
#include "../mem/memory.h"
#include "../mem/ram_memory.h"
#include "../mem/rom_memory.h"
#include "../mpu.h"
#include "../debug/mpu_trace.h"

class FloppyDriveMemoryView : public Memory {
public:
    FloppyDriveMemoryView(RAMMemory *ram, ROMMemory *dos, VIA *via1, VIA *via2) : ram(ram), dos(dos), via1(via1), via2(via2) {}
    uint8_t read(uint16_t addr, bool nonDestructive = false);
    void write(uint16_t addr, uint8_t data);
    RAMMemory *ram;
    ROMMemory *dos;

    VIA *via1;
    VIA *via2;
};

class FloppyDrive : public SerialDevice {
public:
    FloppyDrive(SerialBus *serialBus, const std::vector<uint8_t>& dosROM)
        : mpu(&memoryView), ram(0x800), dos(dosROM),
          memoryView(&ram, &dos, &via1, &via2), serialBus(serialBus),
          mpuTrace(&mpu) {

        serialDeviceName = "floppy";
        serialBus->addDevice(this);
        mpu.reset();
    }

    void tick();

    MPU mpu;
    RAMMemory ram;
    ROMMemory dos;
    VIA via1;
    VIA via2;
    FloppyDriveMemoryView memoryView;

    MPUTrace mpuTrace;

    SerialBus *serialBus;
};
