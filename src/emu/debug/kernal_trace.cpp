#include "kernal_trace.h"

#include <unordered_map>
#include <iostream>

#include "../../gui/text_utils.h"

// src: http://www.ffd2.com/fridge/docs/c64-diss.html
std::unordered_map<uint16_t, std::string> kernelAddrNames = {
    // (B): internal basic functions
    { 0xE394, "(B) RESET routine" },

    // (PK): functions called by KERNAL vectors
    // (K): internal kernel functions
    { 0xE500, "(PK) read base address of I/O devices" },
    { 0xE505, "(PK) read organisation of screen into XY" },
    { 0xE50A, "(PK) read/set XY cursor position" },
    { 0xEA87, "(PK) scan keyboard" },
    { 0xED09, "(PK) send talk on serial bus" },
    { 0xED0C, "(PK) send listen on serial bus" },
    { 0xEDB9, "(PK) read secondary address after listen" },
    { 0xEDC7, "(PK) read secondary address after talk" },
    { 0xEDDD, "(PK) output byte on serial bus" },
    { 0xEDEF, "(PK) send untalk on serial bus" },
    { 0xEDFE, "(PK) send unlisten on serial bus" },
    { 0xEE13, "(PK) input on serial bus" },
    { 0xF13E, "(PK) get a character" },
    { 0xF157, "(PK) input char on current device" },
    { 0xF1CA, "(PK) output char on current device" },
    { 0xF20E, "(PK) set input device" },
    { 0xF250, "(PK) set output device" },
    { 0xF291, "(PK) close a file" },
    { 0xF32F, "(PK) close all channels and files" },
    { 0xF333, "(PK) restore I/O devices to default" },
    { 0xF34A, "(PK) open a file" },
    { 0xF49E, "(PK) load ram from device" },
    { 0xF5DD, "(PK) save ram to device" },
    { 0xF69B, "(PK) increment real time clock" },
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
    { 0xFFCF, "vector for 'input char on current device'" },
    { 0xFFD2, "vector for 'output char on current device'" },
    { 0xFFD5, "vector for 'load ram from device'" },
    { 0xFFD8, "vector for 'save ram to device'" },
    { 0xFFDB, "vector for 'set real time clock'" },
    { 0xFFDE, "vector for 'read real time clock'" },
    { 0xFFE1, "vector for 'check stop key'" },
    { 0xFFE4, "vector for 'get a character'" },
    { 0xFFE7, "vector for 'close all channels and files'" },
    { 0xFFEA, "vector for 'increment real time clock'" },
    { 0xFFED, "vector for 'read organisation of screen into XY'" },
    { 0xFFF0, "vector for 'read/set XY cursor position'" },
    { 0xFFF3, "vector for 'read base address of I/O devices'" },
};

void KernalTrace::tick() {
    if (mpu->T == 0) {
        auto it = kernelAddrNames.find(mpu->PC);
        if (it != kernelAddrNames.end())
            std::cout << "PC:" << toHexStr(it->first) << " " << it->second << std::endl;
    }
}
