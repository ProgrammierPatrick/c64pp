#include "text_utils.h"

char hexChar(int x) {
    if (x > 9) return x + 'A' - 0xA;
    else return x + '0';
}

std::string toHexStr(uint8_t number) {
    char b[3] = "00";
    b[0] = hexChar((number >> 4) & 0xF);
    b[1] = hexChar((number >> 0) & 0xF);
    return std::string(b);
}

std::string toHexStr(uint16_t number) {
    char b[5] = "0000";
    b[0] = hexChar((number >>12) & 0xF);
    b[1] = hexChar((number >> 8) & 0xF);
    b[2] = hexChar((number >> 4) & 0xF);
    b[3] = hexChar((number >> 0) & 0xF);
    return std::string(b);
}

bool isValidHexChar(char c) {
    return c >= '0' && c <= '0' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'z';
}

bool isValidHex8(const std::string& str) {
    return str.size() == 2 && isValidHexChar(str[0]) && isValidHexChar(str[1]);
}

bool isValidHex16(const std::string& str) {
    return str.size() == 4 && isValidHexChar(str[0]) && isValidHexChar(str[1]) && isValidHexChar(str[2]) && isValidHexChar(str[3]);
}

int fromHexChar(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    else if (c >= 'A' && c <= 'F') return c - 'A' + 0xA;
    else if (c >= 'a' && c <= 'f') return c - 'a' + 0xA;
    return 0;
}

uint8_t fromHexStr8(const std::string& str) {
    if (str.size() != 2) return 0;
    return fromHexChar(str[0]) << 4 | fromHexChar(str[1]);
}

uint8_t fromHexStr16(const std::string& str) {
    if (str.size() != 4) return 0;
    return fromHexChar(str[0]) << 12 | fromHexChar(str[1]) << 8 | fromHexChar(str[2]) << 4  | fromHexChar(str[3]);
}

