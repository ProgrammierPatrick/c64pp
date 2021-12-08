#include "keyboard_state.h"

#include <sstream>
#include <iostream>

void KeyboardState::processKeyEvent(QKeyEvent* event, bool pressed) {
    // TODO: also handle RESTORE
    QKeyCombination comb = event->keyCombination();
    int dir = pressed ? 1 : -1;
    for (auto& mapping : keymap) {
        if (mapping.key == comb.key()) {
            if (mapping.mode & ShiftMode::COMBINED_WITH_SHIFT)
                matrixKeyCount[shiftPos] += dir;
            if (mapping.mode & ShiftMode::COMBINED_WITH_CBM)
                matrixKeyCount[cbmPos] += dir;
            if (mapping.mode & ShiftMode::COMBINED_WITH_CTRL)
                matrixKeyCount[ctrlPos] += dir;

            matrixKeyCount[mapping.matrixPos] += dir;
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
            if (row < 0) continue;
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
            else if(keyname == "Escape") key = Qt::Key_Escape;
            else if(keyname == "dead_acute") key = Qt::Key_Dead_Acute;
            else if(keyname == "dead_grave") key = Qt::Key_Dead_Grave;
            else if(keyname == "dead_circumflex") key = Qt::Key_Dead_Circumflex;

            if(key != Qt::Key_unknown) {
                keymap.push_back(Mapping(key, row * 8 + column, ShiftMode(shiftmode)));
            } else {
                std::cout << "Keymap Error: key '" << keyname << "' unknown." << std::endl;
            }
        }
    }
}
