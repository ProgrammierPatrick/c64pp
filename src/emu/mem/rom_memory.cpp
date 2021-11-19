#include "rom_memory.h"

#include <fstream>
#include <iostream>

ROMMemory::ROMMemory(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    auto endPos = file.tellg();
    if (endPos < 0) {
        std::cout << "Error: could not find file '" << filename << "'. exiting." << std::endl;
        exit(1);
    }
    if (endPos < 1) {
        std::cout << "Error: file '" << filename << "' is empty. exiting." << std::endl;
        exit(1);
    }

    data.resize(endPos);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(data.data()), endPos);
}
