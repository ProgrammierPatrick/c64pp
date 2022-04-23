#include "parse_args.h"

#include "functional_test.h"
#include "trace.h"
#include "testsid.h"

#include "../emu/text_utils.h"

#include <iostream>

int help(const char *appName) {
    std::cout << appName << "                                           open GUI application\n";
    std::cout << appName << " [filename.prg]                            open GUI application with PRG file\n";
    std::cout << appName << " args [...]                                open GUI application with Qt command line args\n";
    std::cout << appName << " help                                      show this help\n";
    std::cout << appName << " functional_test [filename] [success_addr] run functional test from https://github.com/Klaus2m5/6502_65C02_functional_tests (binary gets loaded as ram and PC starts at 0x400\n";
    std::cout << appName << " trace [filename] [start_addr]             trace MPU instructions in a format diffable with VICE trace. Filename gets loaded as PRG file after BASIC init\n";
    std::cout << appName << " testsid                                   render sid test audio to raw file\n";
    std::cout << std::endl;
    return 0;
}

int parseArgs(int argc, char** argv) {
    if (argv[1] == std::string("help")) {
        return help(argv[0]);
    }
    else if (argv[1] == std::string("functional_test")) {
        if (argc != 4) {
            std::cout << "Syntax error! expected: " << argv[0] << " functional_test [filename] [success_addr]" << std::endl;
            return -1;
        }
        return functional_test(argv[2], fromHexStr16(argv[3]));
    }
    else if (argv[1] == std::string("trace")) {
        if (argc != 4) {
            std::cout << "Syntax error! expected: " << argv[0] << " trace [filename] [start_addr]" << std::endl;
            return -1;
        }
        return trace(argv[2], fromHexStr16(argv[3]));
    }
    else if (argv[1] == std::string("testsid")) {
        if (argc != 2) {
            std::cout << "Syntax error! expected: " << argv[0] << " testsid" << std::endl;
            return -1;
        }
        return testsid();
    }
    else {
        std::cout << "Unknown command " << argv[1] << "! Call " << argv[0] << " help for help" << std::endl;
        return -1;
    }

}
