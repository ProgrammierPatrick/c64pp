#pragma once

#include <QMainWindow>
#include <QTimer>

#include "c64_runner.h"

#include "mpu_viewer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void updateMessage();

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

