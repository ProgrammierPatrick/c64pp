#pragma once

#include <QMainWindow>

#include "../c64_runner.h"

namespace Ui {
class JoystickWindow;
}

class MainWindow;

class JoystickWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit JoystickWindow(MainWindow *parent, C64Runner* c64Runner);
    ~JoystickWindow();

    void updateUI();

private:
    MainWindow *mainWindow;
    Ui::JoystickWindow *ui;
    C64Runner *c64Runner;
};

