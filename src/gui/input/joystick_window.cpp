#include "joystick_window.h"
#include "ui_joystick_window.h"

JoystickWindow::JoystickWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::JoystickWindow)
{
    ui->setupUi(this);
}

JoystickWindow::~JoystickWindow()
{
    delete ui;
}
