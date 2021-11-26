#include "main_window.h"
#include "ui_main_window.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject::connect(ui->actionHard_Reset, &QAction::triggered, [this]() {
        c64Runner.hardReset();
        frame = 0;
        std::cout << "frame: " << frame << std::endl;
    });

    QObject::connect(ui->actionPause, &QAction::triggered, [this]() {
        if (running) {
            running = false;
            frameTimer.stop();
            ui->actionPause->setText("Start");
        } else {
            running = true;
            frameTimer.start();
            ui->actionPause->setText("Pause");
        }
        std::cout << "running: " << running << std::endl;
    });

    QObject::connect(ui->actionStep, &QAction::triggered, [this]() {
        if (running) {
            running = false;
            frameTimer.stop();
            ui->actionPause->setText("Start");
        }
        c64Runner.singleStepMPU();
        frame++;
        std::cout << "frame: " << frame << std::endl;
    });

    QObject::connect(&frameTimer, &QTimer::timeout, this, [this]() {
        c64Runner.stepFrame();
        frame++;
        std::cout << "frame: " << frame << std::endl;
    });

    frameTimer.setInterval(20); // 50Hz -> 20ms
    frameTimer.start();
}

MainWindow::~MainWindow()
{
    delete ui;
}
