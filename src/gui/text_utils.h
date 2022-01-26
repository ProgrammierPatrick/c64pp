#pragma once

#include <string>
#include <array>

std::string padZeros(std::string str, int len);

std::string toHexStr(uint8_t number);
std::string toHexStr(uint16_t number);

bool isValidHex8(const std::string& str);
bool isValidHex16(const std::string& str);

uint8_t  fromHexStr8(const std::string& str);
uint16_t fromHexStr16(const std::string& str);

char fromPETSCII(uint8_t c);

std::string basicFloatToStr(const uint8_t *data);

// BASIC float in compressed format: EXP M4 M3 M2 M1 with sign as MSB of EXP
std::string basicFloatToStr(std::array<uint8_t,5> data);

// BASIC float in FAC/ARG format: EXP M4 M3 M2 M1 SGN
std::string basicFloatToStr(std::array<uint8_t,6> data);
