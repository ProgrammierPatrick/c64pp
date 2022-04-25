#pragma once

#include <QMainWindow>

#include "c64_runner.h"

namespace Ui {
class VolumeControl;
}

class MainWindow;

class VolumeControl : public QMainWindow
{
    Q_OBJECT

public:
    explicit VolumeControl(MainWindow *parent, C64Runner *c64Runner);
    ~VolumeControl();

private:
    Ui::VolumeControl *ui;
    C64Runner *c64Runner;
    MainWindow *mainWindow;
};

