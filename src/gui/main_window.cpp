#include "main_window.h"
#include "ui_main_window.h"

#include "text_utils.h"
#include "enterhexdialog.h"

#include <iostream>
#include <sstream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto hardReset = [this]() {
        c64Runner.hardReset();
        frame = 0;
        cycle = 0;
        std::cout << "frame: " << frame << std::endl;
        updateUI();
    };
    auto start = [this]() {
        running = true;
        frameTimer.start();
        ui->actionPause->setText("Pause");
        toolbarPauseAction->setText("Pause");
    };
    auto stop = [this]() {
        running = false;
        frameTimer.stop();
        ui->actionPause->setText("Continue");
        toolbarPauseAction->setText("Continue");
    };
    auto pauseUnpause = [this, start, stop]() {
        if (running) {
            stop();
        } else {
            start();
        }
        std::cout << "running: " << running << std::endl;
    };
    auto step = [this, stop]() {
        if (running) {
            stop();
        }
        c64Runner.singleStepMPU();
        cycle++;
        if (cycle % 19704 == 0) frame++;
        std::cout << "frame: " << frame << std::endl;
        updateUI();
    };
    auto stepFrame = [this, stop]() {
        if (running) {
            stop();
        }
        c64Runner.stepFrame();
        cycle += 19704;
        frame++;
        std::cout << "frame: " << frame << std::endl;
        updateUI();
    };
    auto mpuViewer = [this]() {
        if (toolMPUViewer) {
            delete toolMPUViewer;
            toolMPUViewer = nullptr;
        } else {
            toolMPUViewer = new MPUViewer(this, &c64Runner);
            QObject::connect(toolMPUViewer, &QObject::destroyed, [this](QObject *o) { toolMPUViewer = nullptr; });
            toolMPUViewer->setAttribute(Qt::WA_DeleteOnClose, true);
            toolMPUViewer->show();
        }
    };
    auto virtualKeyboard = [this]() {
        if (toolKeyboardWindow) {
            delete toolKeyboardWindow;
            toolKeyboardWindow = nullptr;
        } else {
            toolKeyboardWindow = new KeyboardWindow(this, &c64Runner, this);
            QObject::connect(toolKeyboardWindow, &QObject::destroyed, [this](QObject *o) { toolKeyboardWindow = nullptr; });
            toolKeyboardWindow->setAttribute(Qt::WA_DeleteOnClose, true);

            auto screenSize = qApp->primaryScreen()->size();
            toolKeyboardWindow->move(screenSize.width() / 2 - toolKeyboardWindow->width() / 2, screenSize.height() - toolKeyboardWindow->height() - 100);
            toolKeyboardWindow->show();
        }
    };

    QObject::connect(ui->actionHard_Reset, &QAction::triggered, hardReset);
    QObject::connect(ui->actionPause, &QAction::triggered, pauseUnpause);
    QObject::connect(ui->actionStep, &QAction::triggered, step);
    QObject::connect(ui->actionStep_Frame, &QAction::triggered, stepFrame);

    QObject::connect(ui->actionMPU_Viewer, &QAction::triggered, mpuViewer);
    QObject::connect(ui->actionVirtual_Keyboard, &QAction::triggered, virtualKeyboard);

    QObject::connect(&frameTimer, &QTimer::timeout, this, [this]() {
        c64Runner.stepFrame();
        cycle += 19704;
        frame++;
        std::cout << "frame: " << frame << std::endl;
        updateUI();
    });

    QObject::connect(ui->actionEnter_Hex_Data, &QAction::triggered, [this]() {
        EnterHexDialog diag(this, &c64Runner);
        diag.exec();
        diag.accept();
        updateUI();
    });

    frameTimer.setInterval(20); // 50Hz -> 20ms
    frameTimer.start();
    running = true;

    toolbarPauseAction = ui->toolBar->addAction("Pause", pauseUnpause);
    ui->toolBar->addAction("Step", step);
    ui->toolBar->addAction("Step Frame", stepFrame);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction("Reset", hardReset);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction("MPU..", mpuViewer);
    ui->toolBar->addAction("Keyboard..", virtualKeyboard);

    keyboardWidget = new KeyboardWidget(this, &c64Runner, this);
    keyboardWidget->move(0, 50);

    updateUI();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::keyPressEvent(QKeyEvent* event) {
    c64Runner.keyboard->handleKeyPressEvent(event);
    updateUI();
}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
    c64Runner.keyboard->handleKeyReleaseEvent(event);
    updateUI();
}


void MainWindow::updateUI() {
    if(toolMPUViewer)
        toolMPUViewer->updateC64();
    if(toolKeyboardWindow)
        toolKeyboardWindow->updateUI();

    keyboardWidget->updateUI();

    std::stringstream ss;

    ss << "cycle:" << cycle << " frame:" << frame;
    auto& mpu = c64Runner.c64->mpu;

    ss << " A:" << toHexStr(mpu.A);
    ss << " X:" << toHexStr(mpu.X);
    ss << " Y:" << toHexStr(mpu.Y);
    ss << " S:" << toHexStr(mpu.S);
    ss << " P:" << (mpu.P & 0x80 ? 'N' : '-') << (mpu.P & 0x40 ? 'V' : '-');
    ss << '-' << (mpu.P & 0x10 ? 'B' : '-') << (mpu.P & 0x08 ? 'D' : '-');
    ss << (mpu.P & 0x04 ? 'I' : '-') << (mpu.P & 0x02 ? 'Z' : '-') << (mpu.P & 0x01 ? 'C' : '-');
    ss << " PC:" << toHexStr(mpu.PC);

    ui->statusbar->showMessage(QString::fromStdString(ss.str()));
}
