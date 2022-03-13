#include "joystick_window.h"
#include "ui_joystick_window.h"

#include "../main_window.h"

JoystickWindow::JoystickWindow(MainWindow *parent, C64Runner *c64Runner) :
    QMainWindow(parent),
    mainWindow(parent),
    ui(new Ui::JoystickWindow),
    c64Runner(c64Runner)
{
    ui->setupUi(this);

    setFixedSize(size());
    setWindowTitle("Joysticks");

    QObject::connect(ui->enabledJoy1, &QAbstractButton::clicked, [this](bool checked) {
       this->c64Runner->keyboard->setJoystick1Enabled(checked);
        mainWindow->updateUI();
    });

    QObject::connect(ui->enableJoy2, &QAbstractButton::clicked, [this](bool checked) {
        this->c64Runner->keyboard->setJoystick2Enabled(checked);
        mainWindow->updateUI();
    });
}

JoystickWindow::~JoystickWindow()
{
    delete ui;
}

void JoystickWindow::updateUI() {
    ui->enabledJoy1->setChecked(c64Runner->keyboard->getJoystick1Enabled());
    ui->enableJoy2->setChecked(c64Runner->keyboard->getJoystick2Enabled());

    ui->joystickImg1->setEnabled(c64Runner->keyboard->getJoystick1Enabled());
    ui->joystickImg2->setEnabled(c64Runner->keyboard->getJoystick2Enabled());
}
