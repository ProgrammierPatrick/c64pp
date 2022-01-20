#include "kernal_trace.h"

#include <unordered_map>
#include <iostream>

#include "../../gui/text_utils.h"

const bool includeSpammyNames = false;

// src: http://www.ffd2.com/fridge/docs/c64-diss.html
std::unordered_map<uint16_t, std::string> kernelAddrNames = {
    // (B): internal basic functions
    { 0xE394, "(B) RESET routine" },
    { 0xA408, "(B) array area overflow check" },
    { 0xAB1E, "(B) print string from AY" },
    { 0xBDCD, "(B) print number from AX" },
    { 0xE3BF, "(B) intialisation of basic" },
    { 0xE422, "(B) print BASIC start up messages" },
    { 0xE453, "(B) initialise vectors" },
    { 0xBDDF, "(B) convert number in float accu to string (entry after LDY)" },


    // (PK): functions called by KERNAL vectors
    // (K): internal kernel functions
    { 0xE500, "(PK) read base address of I/O devices" },
    { 0xE505, "(PK) read organisation of screen into XY" },
    { 0xE50A, "(PK) read/set XY cursor position" },
    // spammy: { 0xEA87, "(PK) scan keyboard" },
    { 0xED09, "(PK) send talk on serial bus" },
    { 0xED0C, "(PK) send listen on serial bus" },
    { 0xEDB9, "(PK) read secondary address after listen" },
    { 0xEDC7, "(PK) read secondary address after talk" },
    { 0xEDDD, "(PK) output byte on serial bus" },
    { 0xEDEF, "(PK) send untalk on serial bus" },
    { 0xEDFE, "(PK) send unlisten on serial bus" },
    { 0xEE13, "(PK) input on serial bus" },
    { 0xF13E, "(PK) get a character" },
    // spammy: { 0xF157, "(PK) input char on current device" },
    // spammy: { 0xF1CA, "(PK) output char on current device" },
    { 0xF20E, "(PK) set input device" },
    { 0xF250, "(PK) set output device" },
    { 0xF291, "(PK) close a file" },
    { 0xF32F, "(PK) close all channels and files" },
    { 0xF333, "(PK) restore I/O devices to default" },
    { 0xF34A, "(PK) open a file" },
    { 0xF49E, "(PK) load ram from device" },
    { 0xF5DD, "(PK) save ram to device" },
    // spammy: { 0xF69B, "(PK) increment real time clock" },
    { 0xF6DD, "(PK) read real time clock" },
    { 0xF6ED, "(PK) check stop key" },
    { 0xF6E4, "(PK) set real time clock" },
    { 0xFCE2, "(K) RESET routine"},
    { 0xFD1A, "(PK) set I/O vectors from XY" },
    { 0xFD02, "(K) check for a cartridge" },
    { 0xFD15, "(K) restore I/O vectors; set I/O vectors depending on XY" },
    { 0xFD50, "(PK) initalise memory pointers" },
    { 0xFDA3, "(PK) initalise I/O devices" },
    { 0xFDF9, "(PK) set filename parameters" },
    { 0xFE00, "(PK) set file parameters" },
    { 0xFE07, "(PK) read I/O status word" },
    { 0xFE18, "(PK) control kernal messages" },
    { 0xFE21, "(PK) set timout for serial bus" },
    { 0xFE25, "(PK) read/set top of memory" },
    { 0xFE34, "(PK) read/set bottom of memory" },
    { 0xFF5B, "(PK) initalise screen and keyboard" },

    // KERNAL vectors
    { 0xFF81, "vector for 'initalise screen and keyboard'" },
    { 0xFF84, "vector for 'initalise I/O devices'" },
    { 0xFF87, "vector for 'initalise memory pointers'" },
    { 0xFF8A, "vector for 'restore I/O vectors'" },
    { 0xFF8D, "vector for 'set I/O vectors from XY'" },
    { 0xFF90, "vector for 'control kernal messages'" },
    { 0xFF93, "vector for 'read secondary address after listen'" },
    { 0xFF96, "vector for 'read secondary address after talk'" },
    { 0xFF99, "vector for 'read/set top of memory'" },
    { 0xFF9C, "vector for 'read/set bottom of memory'" },
    { 0xFF9F, "vector for 'scan keyboard'" },
    { 0xFFA2, "vector for 'set timout for serial bus'" },
    { 0xFFA5, "vector for 'input on serial bus'" },
    { 0xFFA8, "vector for 'output byte on serial bus'" },
    { 0xFFAB, "vector for 'send untalk on serial bus'" },
    { 0xFFAE, "vector for 'send unlisten on serial bus'" },
    { 0xFFB1, "vector for 'send listen on serial bus'" },
    { 0xFFB4, "vector for 'send talk on serial bus'" },
    { 0xFFB7, "vector for 'read I/O status word'" },
    { 0xFFBA, "vector for 'set file parameters'" },
    { 0xFFBD, "vector for 'set filename parameters'" },
    { 0xFFC0, "vector for 'open a file'" },
    { 0xFFC3, "vector for 'close a file'" },
    { 0xFFC6, "vector for 'set input device'" },
    { 0xFFC9, "vector for 'set output device'" },
    { 0xFFCC, "vector for 'restore I/O devices to default'" },
    // spammy: { 0xFFCF, "vector for 'input char on current device'" },
    // spammy: { 0xFFD2, "vector for 'output char on current device'" },
    { 0xFFD5, "vector for 'load ram from device'" },
    { 0xFFD8, "vector for 'save ram to device'" },
    { 0xFFDB, "vector for 'set real time clock'" },
    { 0xFFDE, "vector for 'read real time clock'" },
    { 0xFFE1, "vector for 'check stop key'" },
    { 0xFFE4, "vector for 'get a character'" },
    { 0xFFE7, "vector for 'close all channels and files'" },
    // spammy: { 0xFFEA, "vector for 'increment real time clock'" },
    { 0xFFED, "vector for 'read organisation of screen into XY'" },
    { 0xFFF0, "vector for 'read/set XY cursor position'" },
    { 0xFFF3, "vector for 'read base address of I/O devices'" },
};

std::unordered_map<uint16_t, std::string> spammyKernelAddrNames = {
    { 0xEA87, "(PK) scan keyboard" },
    { 0xF157, "(PK) input char on current device" },
    { 0xF1CA, "(PK) output char on current device" },
    { 0xF69B, "(PK) increment real time clock" },
    { 0xFFCF, "vector for 'input char on current device'" },
    { 0xFFD2, "vector for 'output char on current device'" },
    { 0xFFEA, "vector for 'increment real time clock'" },
};

void KernalTrace::tick() {
    if (mpu->T == 0) {
        std::string name = "";
        std::string arg = "";
        bool found = false;

        auto it = kernelAddrNames.find(mpu->PC);
        if (it != kernelAddrNames.end()) {
            found = true;
            name = it->second;
        }

        if constexpr (includeSpammyNames) {
            auto it2 = kernelAddrNames.find(mpu->PC);
            if (it2 != kernelAddrNames.end()) {
                found = true;
                name = it2->second;
            }
        }

        if (found) {

            // extract arguments
            switch(mpu->PC) {
            case 0xAB1E: { // print string from AY
                uint16_t strPtr = mpu->A | (mpu->Y << 8);
                arg = "'";
                for(int i = 0; i < 256; i++) {
                    uint8_t c = mpu->mem->read(strPtr + i);
                    if (c == 0) break;
                    arg += fromPETSCII(c);
                }
                arg += "'";
                break;
            }
            case 0xBDCD: { // print number from AX
                uint16_t num = mpu->A | (mpu->X << 8);
                arg = std::to_string(mpu->mem->read(num) | (mpu->mem->read(num + 1) << 8));
                break;
            }
            case 0xBDDF: { // convert number in float accu to string (entry after LDY)
                std::array<uint8_t,6> data;
                for (int i = 0; i < 6; i++) data[i] = mpu->mem->read(/*FAC*/ 0x61 + i);
                arg = basicFloatToStr(data);
                break;
            } }

            // output to cout
            if (arg != "")
                std::cout << "PC:" << toHexStr(mpu->PC) << " " << name << " (" << arg << ")" << std::endl;
            else
                std::cout << "PC:" << toHexStr(mpu->PC) << " " << name << std::endl;
        }
    }
}
