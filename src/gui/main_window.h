#pragma once

#include "c64_runner.h"
#include "mpu_viewer.h"

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateUI();

private:
    Ui::MainWindow *ui;

    C64Runner c64Runner;
    long frame = 0;
    long cycle = 0;

    float running = true;
    QTimer frameTimer;

    QAction* toolbarPauseAction;

    MPUViewer* toolMPUViewer = nullptr;
};

