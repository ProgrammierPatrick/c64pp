#include "joystick_window.h"
#include "ui_joystick_window.h"

#include "../style.h"
#include "../main_window.h"

JoystickWindow::JoystickWindow(MainWindow *parent, C64Runner *c64Runner) :
    QMainWindow(parent),
    mainWindow(parent),
    ui(new Ui::JoystickWindow),
    c64Runner(c64Runner)
{
    ui->setupUi(this);

    addDarkTitlebar(this);
    setFixedSize(size());
    setWindowTitle("Joysticks");

    joy1Buttons.push_back(ui->keyUp1);
    joy1Buttons.push_back(ui->keyDown1);
    joy1Buttons.push_back(ui->keyLeft1);
    joy1Buttons.push_back(ui->keyRight1);
    joy1Buttons.push_back(ui->keyFire1);
    joy2Buttons.push_back(ui->keyUp2);
    joy2Buttons.push_back(ui->keyDown2);
    joy2Buttons.push_back(ui->keyLeft2);
    joy2Buttons.push_back(ui->keyRight2);
    joy2Buttons.push_back(ui->keyFire2);
    helpLabels.push_back(ui->help0);
    helpLabels.push_back(ui->help1);
    helpLabels.push_back(ui->help2);
    helpLabels.push_back(ui->help3);

    for (QPushButton* b : joy1Buttons) {
        b->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        b->setFlat(true);
    }
    for (QPushButton* b : joy2Buttons) {
        b->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        b->setFlat(true);
    }
    for (QLabel* l : helpLabels) {
        l->setHidden(true);
    }
    ui->plugButton1->setFlat(true);
    ui->plugButton1->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    ui->plugButton2->setFlat(true);
    ui->plugButton2->setFocusPolicy(Qt::FocusPolicy::NoFocus);


    ui->enabledJoy1->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    ui->enabledJoy2->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    ui->keymap->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    ui->help0->setHidden(true);
    ui->help1->setHidden(true);
    ui->help2->setHidden(true);
    ui->help3->setHidden(true);

    QObject::connect(ui->enabledJoy1, &QAbstractButton::clicked, [this](bool checked) {
       this->c64Runner->keyboard->setJoystick1Enabled(checked);
        mainWindow->updateUI();
    });

    QObject::connect(ui->enabledJoy2, &QAbstractButton::clicked, [this](bool checked) {
        this->c64Runner->keyboard->setJoystick2Enabled(checked);
        mainWindow->updateUI();
    });

    QObject::connect(ui->keymap, &QAbstractButton::clicked, [this](bool checked) {
        for (QPushButton* b : joy1Buttons) {
            b->setFlat(!checked);
        }
        for (QPushButton* b : joy2Buttons) {
            b->setFlat(!checked);
        }
        for (QLabel* l : helpLabels) {
            l->setHidden(!checked);
        }
        updateUI();
    });

    QObject::connect(ui->plugButton1, &QAbstractButton::pressed, [this]() {
        auto& keyboard = this->c64Runner->keyboard;
        keyboard->setJoystick1Enabled(!keyboard->getJoystick1Enabled());
        mainWindow->updateUI();
    });
    QObject::connect(ui->plugButton2, &QAbstractButton::pressed, [this]() {
        auto& keyboard = this->c64Runner->keyboard;
        keyboard->setJoystick2Enabled(!keyboard->getJoystick2Enabled());
        mainWindow->updateUI();
    });

    updateUI();
}

void JoystickWindow::setJoyEnabled(bool joy1, bool enabled) {
    auto& buttons = joy1 ? joy1Buttons : joy2Buttons;
    auto& joyImg = joy1 ? ui->joystickImg1 : ui->joystickImg2;
    auto& plugImg = joy1 ? ui->plugImg1 : ui->plugImg2;
    auto& plugText = joy1 ? ui->plugText1 : ui->plugText2;

    for (QPushButton* b : buttons) {
        b->setEnabled(enabled);
    }
    joyImg->setEnabled(enabled);
    plugImg->setEnabled(enabled);
    plugImg->setHidden(!enabled);
    plugText->setHidden(!enabled);
}

JoystickWindow::~JoystickWindow()
{
    delete ui;
}

void JoystickWindow::updateUI() {
    auto& keyboard = *c64Runner->keyboard;

    bool joy1Enabled = keyboard.getJoystick1Enabled();
    bool joy2Enabled = keyboard.getJoystick2Enabled();
    ui->enabledJoy1->setChecked(joy1Enabled);
    ui->enabledJoy2->setChecked(joy2Enabled);

    setJoyEnabled(true, joy1Enabled);
    setJoyEnabled(false, joy2Enabled);

    ui->keyUp1->   setDown(~keyboard.queryJoystick1() & 0x01);
    ui->keyDown1-> setDown(~keyboard.queryJoystick1() & 0x02);
    ui->keyLeft1-> setDown(~keyboard.queryJoystick1() & 0x04);
    ui->keyRight1->setDown(~keyboard.queryJoystick1() & 0x08);
    ui->keyFire1-> setDown(~keyboard.queryJoystick1() & 0x10);
    ui->keyUp2->   setDown(~keyboard.queryJoystick2() & 0x01);
    ui->keyDown2-> setDown(~keyboard.queryJoystick2() & 0x02);
    ui->keyLeft2-> setDown(~keyboard.queryJoystick2() & 0x04);
    ui->keyRight2->setDown(~keyboard.queryJoystick2() & 0x08);
    ui->keyFire2-> setDown(~keyboard.queryJoystick2() & 0x10);

    bool keymapEnabled = ui->keymap->isChecked();
    bool joy1WASD   = keymapEnabled && joy1Enabled;
    bool joy2WASD   = keymapEnabled && joy2Enabled && !joy1Enabled;
    bool joy2Arrows = keymapEnabled && joy2Enabled && joy1Enabled;
    ui->keyUp1->setText(joy1WASD ? "W" : "");
    ui->keyDown1->setText(joy1WASD ? "S" : "");
    ui->keyLeft1->setText(joy1WASD ? "A" : "");
    ui->keyRight1->setText(joy1WASD ? "D" : "");
    ui->keyFire1->setText(joy2Arrows ? "Space" : "");
    ui->keyUp2->setText(joy2Arrows ? "Up" : (joy2WASD ? "W" : ""));
    ui->keyDown2->setText(joy2Arrows ? "Down" : (joy2WASD ? "S" : ""));
    ui->keyLeft2->setText(joy2Arrows ? "Left" : (joy2WASD ? "A" : ""));
    ui->keyRight2->setText(joy2Arrows ? "Right" : (joy2WASD ? "D" : ""));
    ui->keyFire2->setText(joy2Arrows ? "Return" : (joy2WASD ? "Space" : ""));
}


void JoystickWindow::keyPressEvent(QKeyEvent* event) {
    // mainWindow->keyPressEvent(event);
    QApplication::sendEvent(mainWindow, event);
}
void JoystickWindow::keyReleaseEvent(QKeyEvent* event) {
    // mainWindow->keyReleaseEvent(event);
    QApplication::sendEvent(mainWindow, event);
}
