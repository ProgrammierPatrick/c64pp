#include "text_utils.h"

#include <sstream>
#include <cmath>

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
    return c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f';
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

uint16_t fromHexStr16(const std::string& str) {
    if (str.size() != 4) return 0;
    return fromHexChar(str[0]) << 12 | fromHexChar(str[1]) << 8 | fromHexChar(str[2]) << 4  | fromHexChar(str[3]);
}

std::string padZeros(std::string str, int len) {
   while(str.size() < len) str = "0" + str;
   return str;
}

const std::array<char, 256> petsciiToChar = {
    /* 00 */ '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    /* 10 */ '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    /* 20 */ ' ', '!', '"', '#', '$', '%', '&','\'', '(', ')', '*', '+', ',', '-', '.', '/',
    /* 30 */ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
    /* 40 */ '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    /* 50 */ 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '?', ']', '?', '?',
    /* 60 */ '-', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    /* 70 */ 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '+', '?', '|', '?', '?',
    /* 80 */ '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    /* 90 */ '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    /* A0 */ '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    /* B0 */ '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    /* C0 */ '-', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    /* D0 */ 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '+', '?', '|', '?', '?',
    /* E0 */ '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
    /* F0 */ '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
};

char fromPETSCII(uint8_t c) {
    return petsciiToChar[c];
}

std::string basicFloatToStr(std::array<uint8_t,6> data) {
    std::stringstream ss;
    if (data[0] == 0) return "0";
    double value = (data[5] ? -1.0 : 1.0) * (
                (static_cast<double>(data[1]) * pow(2.0, -8.0))
                + (static_cast<double>(data[2]) * pow(2.0, -16.0))
                + (static_cast<double>(data[3]) * pow(2.0, -24.0))
                + (static_cast<double>(data[4]) * pow(2.0, -32.0))
            ) * pow(2.0, data[0] - 128.0);
    ss << value << " [" << toHexStr(data[0]) << " " << toHexStr(data[1]) << " " << toHexStr(data[2])
            << " " << toHexStr(data[3]) << " " + toHexStr(data[4]) << " " << toHexStr(data[5]) << "]";
    return ss.str();
}

std::string basicFloatToStr(std::array<uint8_t,5> data) {
    std::stringstream ss;
    if (data[0] == 0) return "0";
    double value = ((data[1] & 0x80) ? -1.0 : 1.0) * (
                (static_cast<double>(data[1] | 0x80) * pow(2.0, -8.0))
                + (static_cast<double>(data[2]) * pow(2.0, -16.0))
                + (static_cast<double>(data[3]) * pow(2.0, -24.0))
                + (static_cast<double>(data[4]) * pow(2.0, -32.0))
            ) * pow(2.0, data[0] - 128.0);
    ss << value << " [" << toHexStr(data[0]) << " " << toHexStr(data[1]) << " " << toHexStr(data[2])
            << " " << toHexStr(data[3]) << " " + toHexStr(data[4]) << "]";
    return ss.str();
}
