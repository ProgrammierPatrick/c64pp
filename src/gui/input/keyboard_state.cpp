#include "keyboard_state.h"

#include <QMetaEnum>

#include <sstream>
#include <iostream>
#include <set>

bool KeyboardState::isSuitableBinding(const KeyboardState::Mapping& mapping, bool osCtrl, bool osShift, bool osAltGr) {
    return mapping.mode == 0 && !osShift
        || (mapping.mode & (ShiftMode::SHIFT_OR_NOT | ShiftMode::IS_LEFT_SHIFT | ShiftMode::IS_LEFT_CBM | ShiftMode::IS_LEFT_CTRL | ShiftMode::IS_RIGHT_SHIFT))
        || (mapping.mode & ShiftMode::CTRL_REQUIRED) && osCtrl
        || (mapping.mode & (ShiftMode::SHIFT_REQUIRED | ShiftMode::DESHIFT)) && osShift
        || (mapping.mode & ShiftMode::COMBINED_WITH_SHIFT)
        || (mapping.mode & ShiftMode::ALT_GR_REQUIRED) && osAltGr;
}

void KeyboardState::processKeyEvent(QKeyEvent* event, bool pressed) {
    // TODO: keys get stuck when modifier key gets changed when pressing button which is not SHIFT_OR_NOT.
    //       In this case, the key will not get released because its shifting requirements are not satisfied with new modifier keys.
    //       Possible fix: track currently held down Qt keys in separate datastraucture and compute C64 keys from scratch each update.

    QKeyCombination comb = event->keyCombination();
    auto key = comb.key();
    int dir = pressed ? 1 : -1;
    auto setJoystick = [pressed,this](bool first, int bit) {
        uint8_t& reg = first ? joystick1State : joystick2State;
        if (pressed) reg &= ~(1 << bit);
        else         reg |=   1 << bit;
    };

    // update modifier keys. They can change if modifiers change or if dedicated key event is processed.
    bool newOSCtrl = comb.keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier);
    bool newOSShift = comb.keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier);
    bool newOSAlt = comb.keyboardModifiers().testFlag(Qt::KeyboardModifier::AltModifier);
    if (comb.key() == Qt::Key_Control) newOSCtrl = pressed;
    if (comb.key() == Qt::Key_Shift) newOSShift = pressed;
    if (comb.key() == Qt::Key_Alt) newOSAlt = pressed;
    if (!currentOSCtrl && newOSCtrl) matrixKeyCount[ctrlPos]++;
    if (currentOSCtrl && !newOSCtrl) matrixKeyCount[ctrlPos]--;
    if (!currentOSShift && newOSShift) matrixKeyCount[shiftPos]++;
    if (currentOSShift && !newOSShift) matrixKeyCount[shiftPos]--;
    if (!currentOSAlt && newOSAlt) matrixKeyCount[ctrlPos] -= 100; // disable ctrl while alt is pressed. This fixes the weird handling of AltGr by Qt where AltGr is Ctrl + Alt
    if (currentOSAlt && !newOSAlt) matrixKeyCount[ctrlPos] += 100;
    currentOSCtrl = newOSCtrl;
    currentOSShift = newOSShift;
    currentOSAlt = newOSAlt;

    // this key is a shortcut, do not process it
    if (currentOSAlt && !currentOSCtrl)
        return;

    // joystick keymap: (1) WASD+Space, (2) WASD+Space + Arrows+Enter
    if (joystick1Enabled && joystick2Enabled) {
        if (key == Qt::Key_W)     { setJoystick(true, 0); return; }
        if (key == Qt::Key_S)     { setJoystick(true, 1); return; }
        if (key == Qt::Key_A)     { setJoystick(true, 2); return; }
        if (key == Qt::Key_D)     { setJoystick(true, 3); return; }
        if (key == Qt::Key_Space) { setJoystick(true, 4); return; }
        if (key == Qt::Key_Up)    { setJoystick(false, 0); return; }
        if (key == Qt::Key_Down)  { setJoystick(false, 1); return; }
        if (key == Qt::Key_Left)  { setJoystick(false, 2); return; }
        if (key == Qt::Key_Right) { setJoystick(false, 3); return; }
        if (key == Qt::Key_Return){ setJoystick(false, 4); return; }
    }
    else if (joystick1Enabled || joystick2Enabled) {
        bool joy = joystick1Enabled;
        if (key == Qt::Key_W)     { setJoystick(joy, 0); return; }
        if (key == Qt::Key_S)     { setJoystick(joy, 1); return; }
        if (key == Qt::Key_A)     { setJoystick(joy, 2); return; }
        if (key == Qt::Key_D)     { setJoystick(joy, 3); return; }
        if (key == Qt::Key_Space) { setJoystick(joy, 4); return; }
    }

    for (const auto& mapping : keymap) {
        // modifier keys are handled separately above
        if (mapping.matrixPos == shiftPos || mapping.matrixPos == rightShiftPos || mapping.matrixPos == ctrlPos)
            continue;

        if (mapping.key == comb.key()) {
            if (isSuitableBinding(mapping, currentOSCtrl, currentOSShift, currentOSAlt)) {

                if (mapping.mode & ShiftMode::COMBINED_WITH_SHIFT) {
                    matrixKeyCount[shiftPos] += dir;
                    matrixKeyCount[rightShiftPos] += dir;
                } if (mapping.mode & ShiftMode::COMBINED_WITH_CBM)
                    matrixKeyCount[cbmPos] += dir;
                if (mapping.mode & ShiftMode::COMBINED_WITH_CTRL)
                    matrixKeyCount[ctrlPos] += dir;
                if (mapping.mode & ShiftMode::DESHIFT) {
                    matrixKeyCount[shiftPos] -= 100 * dir; // very high number to ensure priority
                    matrixKeyCount[rightShiftPos] -= 100 * dir;
                }

                matrixKeyCount[mapping.matrixPos] += dir;
            }
        }
    }
    for (auto& key : restoreKeymap) {
        if (key == comb.key()) {
            restoreKeyCount += dir;
        }
    }
}

void KeyboardState::loadKeymap(const std::string &viceKeymapText) {
    std::stringstream ss(viceKeymapText);
    std::string s;
    while (ss >> s) {
        if(s.size() == 0)
            continue;
        else if(s[0] == '#') // comments
            std::getline(ss, s);
        else if (s[0] == '!') // additional commands, not needed here
            std::getline(ss, s);
        else {
            int row, column, shiftmode;
            std::string keyname = s;
            ss >> row >> column;
            bool isRestore = false;
            if (row < 0) {
                if (row == -3) isRestore = true;
                else continue;
            }
            ss >> shiftmode;

            Qt::Key key = Qt::Key_unknown;
            if(keyname.size() == 1 && keyname[0] >= '0' && keyname[0] <= '9')
                key = static_cast<Qt::Key>(Qt::Key_0 + (keyname[0] - '0'));
            else if(keyname.size() == 1 && keyname[0] >= 'A' && keyname[0] <= 'Z')
                key = static_cast<Qt::Key>(Qt::Key_A + (keyname[0] - 'A'));
            else if(keyname.size() == 1 && keyname[0] >= 'a' && keyname[0] <= 'z')
                key = static_cast<Qt::Key>(Qt::Key_A + (keyname[0] - 'a'));
            else if(keyname.size() == 2 && keyname[0] == 'F' && keyname[1] >= '1' && keyname[1] <= '9')
                key = static_cast<Qt::Key>(Qt::Key_F1 + (keyname[1] - '1'));
            else if(keyname == "F10") key = Qt::Key_F10;
            else if(keyname == "F11") key = Qt::Key_F11;
            else if(keyname == "F12") key = Qt::Key_F12;
            else if(keyname == "BackSpace") key = Qt::Key_Backspace;
            else if(keyname == "Delete") key = Qt::Key_Delete;
            else if(keyname == "Insert") key = Qt::Key_Insert;
            else if(keyname == "Return") key = Qt::Key_Return;
            else if(keyname == "Right") key = Qt::Key_Right;
            else if(keyname == "Left") key = Qt::Key_Left;
            else if(keyname == "Up") key = Qt::Key_Up;
            else if(keyname == "Down") key = Qt::Key_Down;
            else if(keyname == "Page_Up") key = Qt::Key_PageUp;
            else if(keyname == "Page_Down") key = Qt::Key_PageDown;
            else if(keyname == "section") key = Qt::Key_section; // [§]
            else if(keyname == "numbersign") key = Qt::Key_NumberSign; // [#]
            else if(keyname == "dollar") key = Qt::Key_Dollar;
            else if(keyname == "Shift_L") key = Qt::Key_Shift; // sadly no distiction in qt
            else if(keyname == "Caps_Lock") key = Qt::Key_CapsLock;
            else if(keyname == "percent") key = Qt::Key_Percent;
            else if(keyname == "ampersand") key = Qt::Key_Ampersand;
            else if(keyname == "apostrophe") key = Qt::Key_Apostrophe; // [']
            else if(keyname == "parenleft") key = Qt::Key_ParenLeft;
            else if(keyname == "parenright") key = Qt::Key_ParenRight;
            else if(keyname == "plus") key = Qt::Key_Plus;
            else if(keyname == "minus") key = Qt::Key_Minus;
            else if(keyname == "grave") key = Qt::Key_QuoteLeft;
            else if(keyname == "period") key = Qt::Key_Period;
            else if(keyname == "greater") key = Qt::Key_Greater;
            else if(keyname == "colon") key = Qt::Key_Colon;
            else if(keyname == "Adiaeresis") key = Qt::Key_Adiaeresis;
            else if(keyname == "Odiaeresis") key = Qt::Key_Odiaeresis;
            else if(keyname == "Udiaeresis") key = Qt::Key_Udiaeresis;
            else if(keyname == "adiaeresis") key = Qt::Key_Adiaeresis;
            else if(keyname == "odiaeresis") key = Qt::Key_Odiaeresis;
            else if(keyname == "udiaeresis") key = Qt::Key_Udiaeresis;
            else if(keyname == "bracketleft") key = Qt::Key_BracketLeft;
            else if(keyname == "bracketright") key = Qt::Key_BracketRight;
            else if(keyname == "at") key = Qt::Key_At; // [@]
            else if(keyname == "comma") key = Qt::Key_Comma;
            else if(keyname == "less") key = Qt::Key_Less;
            else if(keyname == "bar") key = Qt::Key_Bar;
            else if(keyname == "brokenbar") key = Qt::Key_brokenbar;
            else if(keyname == "acute") key = Qt::Key_acute;
            else if(keyname == "asterisk") key = Qt::Key_Asterisk;
            else if(keyname == "semicolon") key = Qt::Key_Semicolon;
            else if(keyname == "Home") key = Qt::Key_Home;
            else if(keyname == "End") key = Qt::Key_End;
            else if(keyname == "backslash") key = Qt::Key_Backslash;
            else if(keyname == "Shift_R") key = Qt::Key_Shift;
            else if(keyname == "equal") key = Qt::Key_Equal;
            else if(keyname == "asciicircum") key = Qt::Key_AsciiCircum;
            else if(keyname == "degree") key = Qt::Key_degree;
            else if(keyname == "mu") key = Qt::Key_mu;
            else if(keyname == "slash") key = Qt::Key_Slash;
            else if(keyname == "question") key = Qt::Key_Question;
            else if(keyname == "exclam") key = Qt::Key_Exclam;
            else if(keyname == "underscore") key = Qt::Key_Underscore;
            else if(keyname == "asciitilde") key = Qt::Key_AsciiTilde;
            else if(keyname == "ssharp") key = Qt::Key_ssharp;
            else if(keyname == "Control_L") key = Qt::Key_Control;
            else if(keyname == "quotedbl") key = Qt::Key_QuoteDbl;
            else if(keyname == "space") key = Qt::Key_Space;
            else if(keyname == "Tab") key = Qt::Key_Tab;
            else if(keyname == "ISO_Left_Tab") key = Qt::Key_Backtab;
            else if(keyname == "Escape") key = Qt::Key_Escape;
            else if(keyname == "dead_acute") key = Qt::Key_Dead_Acute;
            else if(keyname == "dead_grave") key = Qt::Key_Dead_Grave;
            else if(keyname == "dead_circumflex") key = Qt::Key_Dead_Circumflex;

            if(key != Qt::Key_unknown) {
                if (row >= 0)
                    keymap.push_back(Mapping(key, row * 8 + column, ShiftMode(shiftmode)));
                if (isRestore)
                    restoreKeymap.push_back(key);
            } else {
                std::cout << "Keymap Error: key '" << keyname << "' unknown." << std::endl;
            }
        }
    }
}

std::string KeyboardState::getBindingName(int row, int col, bool shifted) {
    std::set<std::string> result;
    QMetaEnum metaEnum = QMetaEnum::fromType<Qt::Key>();

    for(const auto& binding : this->keymap) {
        if (binding.matrixPos == row * 8 + col) {
            bool ctrl = matrixKeyCount[ctrlPos] > 0;
            bool shift = matrixKeyCount[shiftPos] > 0;
            if (false
                || (binding.mode & (ShiftMode::SHIFT_OR_NOT | ShiftMode::IS_LEFT_SHIFT | ShiftMode::IS_LEFT_CBM | ShiftMode::IS_LEFT_CTRL | ShiftMode::IS_RIGHT_SHIFT))
                || (binding.mode & ShiftMode::CTRL_REQUIRED) && ctrl
                || (binding.mode & (ShiftMode::SHIFT_REQUIRED | ShiftMode::DESHIFT) && shift)) {

                result.insert(std::string(metaEnum.valueToKey(binding.key)).substr(4)); // substr(4): Key_Space to Space
            }
        }
    }
    std::string str = "";
    for (int i = 0; const auto& s : result) {
        str += s + (i == result.size() - 1 ? "" : "\n");
        i++;
    }
    return str;
}

std::string KeyboardState::getRestoreBindingName() {
    QMetaEnum metaEnum = QMetaEnum::fromType<Qt::Key>();
    return std::string(metaEnum.valueToKey(restoreKeymap.front())).substr(4);
}
