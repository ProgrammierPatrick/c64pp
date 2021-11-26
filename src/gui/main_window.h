#pragma once

#include <QMainWindow>
#include <QTimer>

#include "c64_runner.h"

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
    Ui::MainWindow *ui;

    C64Runner c64Runner;
    long frame = 0;
    float running = true;
    QTimer frameTimer;
};

