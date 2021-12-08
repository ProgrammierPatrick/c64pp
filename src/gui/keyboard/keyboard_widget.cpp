#include "keyboard_widget.h"
#include "ui_keyboard_widget.h"

#include "../main_window.h"

KeyboardWidget::KeyboardWidget(QWidget *parent, C64Runner *c64Runner, MainWindow *mainWindow) :
    QWidget(parent),
    ui(new Ui::KeyboardWidget),
    c64Runner(c64Runner), mainWindow(mainWindow)
{
    ui->setupUi(this);

    initVirtualKeyboard();

    // setup normal keys
    for (auto& [button, row, col] : virtualKeyboard) {
        int r = row;
        int c = col;
        QObject::connect(button, &QPushButton::pressed,  [this, r, c](){
            this->c64Runner->keyboard->pressKey(r, c);
            this->mainWindow->updateUI();
        });
        QObject::connect(button, &QPushButton::released, [this, r, c](){
            this->c64Runner->keyboard->releaseKey(r, c);
            this->mainWindow->updateUI();
        });
        button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        button->setFlat(true);
    }

    // setup restore
    QObject::connect(ui->keyRestore, &QPushButton::pressed, [this]() {
        this->c64Runner->keyboard->pressRestore();
        this->mainWindow->updateUI();
    });
    QObject::connect(ui->keyRestore, &QPushButton::released, [this]() {
        this->c64Runner->keyboard->releaseRestore();
        this->mainWindow->updateUI();
    });
    ui->keyRestore->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    ui->keyRestore->setFlat(true);
}

KeyboardWidget::~KeyboardWidget()
{
    delete ui;
}

void KeyboardWidget::updateUI() {
    for (auto& [button, row, col] : virtualKeyboard) {
        button->setDown(c64Runner->keyboard->querySingleKey(row, col));
    }
}

void KeyboardWidget::initVirtualKeyboard() {
    virtualKeyboard.push_back(std::make_tuple(ui->keyDel, 0, 0));
    virtualKeyboard.push_back(std::make_tuple(ui->keyReturn, 0, 1));
    virtualKeyboard.push_back(std::make_tuple(ui->keyCursorRight, 0, 2));
    virtualKeyboard.push_back(std::make_tuple(ui->keyF7, 0, 3));
    virtualKeyboard.push_back(std::make_tuple(ui->keyF1, 0, 4));
    virtualKeyboard.push_back(std::make_tuple(ui->keyF3, 0, 5));
    virtualKeyboard.push_back(std::make_tuple(ui->keyF5, 0, 6));
    virtualKeyboard.push_back(std::make_tuple(ui->keyCursorDown, 0, 7));

    virtualKeyboard.push_back(std::make_tuple(ui->key3, 1, 0));
    virtualKeyboard.push_back(std::make_tuple(ui->keyW, 1, 1));
    virtualKeyboard.push_back(std::make_tuple(ui->keyA, 1, 2));
    virtualKeyboard.push_back(std::make_tuple(ui->key4, 1, 3));
    virtualKeyboard.push_back(std::make_tuple(ui->keyZ, 1, 4));
    virtualKeyboard.push_back(std::make_tuple(ui->keyS, 1, 5));
    virtualKeyboard.push_back(std::make_tuple(ui->keyE, 1, 6));
    virtualKeyboard.push_back(std::make_tuple(ui->keyLShift, 1, 7));

    virtualKeyboard.push_back(std::make_tuple(ui->key5, 2, 0));
    virtualKeyboard.push_back(std::make_tuple(ui->keyR, 2, 1));
    virtualKeyboard.push_back(std::make_tuple(ui->keyD, 2, 2));
    virtualKeyboard.push_back(std::make_tuple(ui->key6, 2, 3));
    virtualKeyboard.push_back(std::make_tuple(ui->keyC, 2, 4));
    virtualKeyboard.push_back(std::make_tuple(ui->keyF, 2, 5));
    virtualKeyboard.push_back(std::make_tuple(ui->keyT, 2, 6));
    virtualKeyboard.push_back(std::make_tuple(ui->keyX, 2, 7));

    virtualKeyboard.push_back(std::make_tuple(ui->key7, 3, 0));
    virtualKeyboard.push_back(std::make_tuple(ui->keyY, 3, 1));
    virtualKeyboard.push_back(std::make_tuple(ui->keyG, 3, 2));
    virtualKeyboard.push_back(std::make_tuple(ui->key8, 3, 3));
    virtualKeyboard.push_back(std::make_tuple(ui->keyB, 3, 4));
    virtualKeyboard.push_back(std::make_tuple(ui->keyH, 3, 5));
    virtualKeyboard.push_back(std::make_tuple(ui->keyU, 3, 6));
    virtualKeyboard.push_back(std::make_tuple(ui->keyV, 3, 7));

    virtualKeyboard.push_back(std::make_tuple(ui->key9, 4, 0));
    virtualKeyboard.push_back(std::make_tuple(ui->keyI, 4, 1));
    virtualKeyboard.push_back(std::make_tuple(ui->keyJ, 4, 2));
    virtualKeyboard.push_back(std::make_tuple(ui->key0, 4, 3));
    virtualKeyboard.push_back(std::make_tuple(ui->keyM, 4, 4));
    virtualKeyboard.push_back(std::make_tuple(ui->keyK, 4, 5));
    virtualKeyboard.push_back(std::make_tuple(ui->keyO, 4, 6));
    virtualKeyboard.push_back(std::make_tuple(ui->keyN, 4, 7));

    virtualKeyboard.push_back(std::make_tuple(ui->keyPlus, 5, 0));
    virtualKeyboard.push_back(std::make_tuple(ui->keyP, 5, 1));
    virtualKeyboard.push_back(std::make_tuple(ui->keyL, 5, 2));
    virtualKeyboard.push_back(std::make_tuple(ui->keyMinus, 5, 3));
    virtualKeyboard.push_back(std::make_tuple(ui->keyPeriod, 5, 4));
    virtualKeyboard.push_back(std::make_tuple(ui->keyColon, 5, 5));
    virtualKeyboard.push_back(std::make_tuple(ui->keyAt, 5, 6));
    virtualKeyboard.push_back(std::make_tuple(ui->keyComma, 5, 7));

    virtualKeyboard.push_back(std::make_tuple(ui->keyPound, 6, 0));
    virtualKeyboard.push_back(std::make_tuple(ui->keyAsterisk, 6, 1));
    virtualKeyboard.push_back(std::make_tuple(ui->keySemicolon, 6, 2));
    virtualKeyboard.push_back(std::make_tuple(ui->keyHome, 6, 3));
    virtualKeyboard.push_back(std::make_tuple(ui->keyRShift, 6, 4));
    virtualKeyboard.push_back(std::make_tuple(ui->keyEquals, 6, 5));
    virtualKeyboard.push_back(std::make_tuple(ui->keyUp, 6, 6));
    virtualKeyboard.push_back(std::make_tuple(ui->keySlash, 6, 7));

    virtualKeyboard.push_back(std::make_tuple(ui->key1, 7, 0));
    virtualKeyboard.push_back(std::make_tuple(ui->keyLeft, 7, 1));
    virtualKeyboard.push_back(std::make_tuple(ui->keyCtrl, 7, 2));
    virtualKeyboard.push_back(std::make_tuple(ui->key2, 7, 3));
    virtualKeyboard.push_back(std::make_tuple(ui->keySpace, 7, 4));
    virtualKeyboard.push_back(std::make_tuple(ui->keyCommodore, 7, 5));
    virtualKeyboard.push_back(std::make_tuple(ui->keyQ, 7, 6));
    virtualKeyboard.push_back(std::make_tuple(ui->keyRunStop, 7, 7));
}
