#pragma once

#include <string>

std::string toHexStr(uint8_t number);
std::string toHexStr(uint16_t number);

bool isValidHex8(const std::string& str);
bool isValidHex16(const std::string& str);

uint8_t fromHexStr8(const std::string& str);
uint8_t fromHexStr16(const std::string& str);
