#include "rom_memory.h"

#include <fstream>

RomMemory::RomMemory(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    auto endPos = file.tellg();

    data.resize(endPos);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(data.data()), endPos);
}
