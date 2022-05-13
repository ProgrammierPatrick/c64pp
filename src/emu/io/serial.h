#pragma once

#include <cstdint>
#include <vector>
#include <string>

class SerialDevice {
public:
    bool pullAttention = false;
    bool pullClock = false;
    bool pullData = false;
    std::string serialDeviceName = "";
};

class SerialBus {
public:
    void addDevice(SerialDevice *device) { devices.push_back(device); }

    bool getAttention() { for(auto d : devices) if (d->pullAttention) return false; return true; }
    bool getClock()     { for(auto d : devices) if (d->pullClock)     return false; return true; }
    bool getData()      { for(auto d : devices) if (d->pullData)      return false; return true; }

    std::vector<SerialDevice*> devices;
};
