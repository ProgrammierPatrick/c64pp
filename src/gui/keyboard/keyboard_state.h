#pragma once

#include "../../emu/io/keyboard.h"

#include <QKeyEvent>

#include <string>
#include <array>

// holds keymap file and current key matrix state
class KeyboardState : public Keyboard {
public:
    KeyboardState(const std::string& viceKeymapText) {
        loadKeymap(viceKeymapText);
    }

    void loadKeymap(const std::string& viceKeymapText);

    uint8_t query(uint8_t mask) override {
        uint8_t result = 0xFF;
        for (int j = 0; j < 8; j++)
            if (!(mask & (1 << j)))
                for (int i = 0; i < 8; i++)
                    if (matrixKeyCount[j * 8 + i] > 0)
                        result &= ~(1 << i);
        return result;
    }
    bool queryRestore() { return restoreKeyCount > 0; }
    bool querySingleKey(int row, int col) { return matrixKeyCount[row * 8 + col] > 0; }

    void pressKey  (int row, int col) { matrixKeyCount[row * 8 + col]++; }
    void releaseKey(int row, int col) { matrixKeyCount[row * 8 + col]--; }
    void pressRestore  () { restoreKeyCount++; }
    void releaseRestore() { restoreKeyCount--; }
    void handleKeyPressEvent(QKeyEvent* event) { processKeyEvent(event, true); }
    void handleKeyReleaseEvent(QKeyEvent* event) { processKeyEvent(event, false); }

    void resetPressedKeys() {
        for (auto& m : matrixKeyCount)
            m = 0;
        restoreKeyCount = 0;
    }

private:
    void processKeyEvent(QKeyEvent* event, bool pressed);

    const int shiftPos = 1 * 8 + 7;
    const int cbmPos = 5 * 8 + 5;
    const int ctrlPos = 7 * 8 + 2;

    // counts from how many different keys the matrix cell is activated. value > 0 means pressed
    std::array<int, 64> matrixKeyCount { 0 };

    // counts from how many different keys restore is activated. value > 0 means pressed
    int restoreKeyCount = 0;

    enum ShiftMode {
        NOT_SHIFTED = 0,
        COMBINED_WITH_SHIFT = 1,
        IS_LEFT_SHIFT = 2,
        IS_RIGHT_SHIFT = 4,
        SHIFT_OR_NOT = 8,
        DESHIFT = 16,
        ALLOW_OTHER_UNIMPLEMENTED = 32,
        IS_SHIFT_LOCK_ON = 64,
        SHIFT_REQUIRED = 128,
        ALT_MAPPING = 256,
        ALT_GR_REQUIRED = 512,
        CTRL_REQUIRED = 1024,
        COMBINED_WITH_CBM = 2048,
        COMBINED_WITH_CTRL = 4096,
        IS_LEFT_CBM = 8192,
        IS_LEFT_CTRL = 16384
    };

    struct Mapping {
        Qt::Key key;
        int matrixPos;
        ShiftMode mode;
        Mapping(Qt::Key key, int matrixPos, ShiftMode mode) : key(key), matrixPos(matrixPos), mode(mode) {}
    };

    std::vector<Mapping> keymap;

};
