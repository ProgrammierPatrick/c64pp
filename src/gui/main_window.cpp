#include "main_window.h"
#include "ui_main_window.h"

#include "text_utils.h"
#include "file/enterhexdialog.h"
#include "file/prg_loader.h"
#include "file/extract_prg.h"

#include <iostream>
#include <sstream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    frameTimer.setTimerType(Qt::TimerType::PreciseTimer);

    auto tickFPS = [this]() {
        auto t = std::chrono::system_clock::now();
        float newFPS = 1.f / std::chrono::duration<float>(t - lastFrameTime).count();
        currentFPS = 0.9 * currentFPS + 0.1 * newFPS; // exponential smoothing
        lastFrameTime = t;
    };

    auto hardReset = [this]() {
        c64Runner.hardReset();
        frame = 0;
        cycle = 0;
        updateUI();
    };
    auto start = [this]() {
        running = true;
        c64Runner.c64->breakPoints.resetBreakpoints();
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
        c64Runner.c64->breakPoints.resetBreakpoints();
        try {
            c64Runner.singleStepMPU();
        }
        catch (std::runtime_error&) { }
        catch (BreakPointException&) { }
        cycle++;
        if (c64Runner.c64->vic.y == 0 && c64Runner.c64->vic.cycleInLine == 1) frame++;
        updateUI();
    };
    auto stepInstruction = [this, stop]() {
        if (running) {
            stop();
        }

        int usedCycles = 0;

        try {
            usedCycles = c64Runner.stepInstruction();
        } catch (std::runtime_error&) {
            stop();
        } catch (BreakPointException&) {
            stop();
        }

        cycle += usedCycles;
        if (c64Runner.c64->vic.y == 0 && c64Runner.c64->vic.cycleInLine < usedCycles) frame++;

        updateUI();
    };
    auto stepLine = [this, stop]() {
        if (running) {
            stop();
        }

        try {
            c64Runner.c64->breakPoints.resetBreakpoints();
            cycle += c64Runner.stepLine();
            if (c64Runner.c64->vic.y == 0) frame++;
        } catch (std::runtime_error&) {
            stop();
        } catch (BreakPointException&) {
            stop();
        }

        updateUI();
    };
    auto stepFrame = [this, stop]() {
        if (running) {
            stop();
        }

        try {
            c64Runner.c64->breakPoints.resetBreakpoints();
            cycle += c64Runner.stepFrame();
            frame++;
        } catch (std::runtime_error&) {
            stop();
        } catch (BreakPointException&) {
            stop();
        }

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
    auto ciaViewer = [this]() {
        if (toolCIAViewer) {
            delete toolCIAViewer;
            toolCIAViewer = nullptr;
        } else {
            toolCIAViewer = new CIAViewer(this, &c64Runner);
            QObject::connect(toolCIAViewer, &QObject::destroyed, [this](QObject *o) { toolCIAViewer = nullptr; });
            toolCIAViewer->setAttribute(Qt::WA_DeleteOnClose, true);
            toolCIAViewer->show();
        }
    };
    auto vicViewer = [this]() {
        if (toolVICViewer) {
            delete toolVICViewer;
            toolVICViewer = nullptr;
        } else {
            toolVICViewer = new VICViewer(this, &c64Runner);
            QObject::connect(toolVICViewer, &QObject::destroyed, [this](QObject *o) { toolVICViewer = nullptr; });
            toolVICViewer->setAttribute(Qt::WA_DeleteOnClose, true);
            toolVICViewer->show();
        }
    };
    auto ramViewer = [this]() {
        if (toolRAMViewer) {
            delete toolRAMViewer;
            toolRAMViewer = nullptr;
        } else {
            toolRAMViewer = new RAMViewer(this, &c64Runner);
            QObject::connect(toolRAMViewer, &QObject::destroyed, [this](QObject *o) { toolRAMViewer = nullptr; });
            toolRAMViewer->setAttribute(Qt::WA_DeleteOnClose, true);
            toolRAMViewer->show();
        }
    };
    auto breakpointEditor = [this]() {
        if (toolBreakpointEditor) {
            delete toolBreakpointEditor;
            toolBreakpointEditor = nullptr;
        } else {
            toolBreakpointEditor = new BreakpointEditor(this, &c64Runner);
            QObject::connect(toolBreakpointEditor, &QObject::destroyed, [this](QObject*) { toolBreakpointEditor = nullptr; });
            toolBreakpointEditor->setAttribute(Qt::WA_DeleteOnClose, true);
            toolBreakpointEditor->show();
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
    auto joystickWindow = [this]() {
        if (toolJoystickWindow) {
            delete toolJoystickWindow;
            toolJoystickWindow = nullptr;
        } else {
            toolJoystickWindow = new JoystickWindow(this, &c64Runner);
            QObject::connect(toolJoystickWindow, &QObject::destroyed, [this](QObject *o) { toolJoystickWindow = nullptr; });
            toolJoystickWindow->setAttribute(Qt::WA_DeleteOnClose, true);
            toolJoystickWindow->show();
        }
    };

    QObject::connect(ui->actionHard_Reset, &QAction::triggered, hardReset);
    QObject::connect(ui->actionPause, &QAction::triggered, pauseUnpause);
    QObject::connect(ui->actionStep, &QAction::triggered, step);
    QObject::connect(ui->actionStep_Instruction, &QAction::triggered, stepInstruction);
    QObject::connect(ui->actionStep_Line, &QAction::triggered, stepLine);
    QObject::connect(ui->actionStep_Frame, &QAction::triggered, stepFrame);

    QObject::connect(ui->actionMPU_Viewer, &QAction::triggered, mpuViewer);
    QObject::connect(ui->actionRAM_Viewer, &QAction::triggered, ramViewer);
    QObject::connect(ui->actionCIA_Viewer, &QAction::triggered, ciaViewer);
    QObject::connect(ui->actionVIC_Viewer, &QAction::triggered, vicViewer);
    QObject::connect(ui->actionVirtual_Keyboard, &QAction::triggered, virtualKeyboard);
    QObject::connect(ui->actionVirtual_Joysticks, &QAction::triggered, joystickWindow);
    QObject::connect(ui->actionBreakpoint_Editor, &QAction::triggered, breakpointEditor);

    QObject::connect(&frameTimer, &QTimer::timeout, this, [this, stop, tickFPS]() {
        try {
            cycle += c64Runner.stepFrame();
            tickFPS();
        } catch (std::runtime_error&) {
            stop();
        } catch (BreakPointException&) {
            stop();
        }
        frame++;
        updateUI();
    });

    QObject::connect(ui->actionEnter_Hex_Data, &QAction::triggered, [this]() {
        EnterHexDialog diag(this, &c64Runner);
        diag.exec();
        updateUI();
    });

    QObject::connect(ui->actionOpen_PRG, &QAction::triggered, [this]() {
        PRGLoader::openPRGFile(this, &c64Runner);
        updateUI();
    });

    QObject::connect(ui->actionExtract_RAM_to_File, &QAction::triggered, [this]() {
        ExtractPRG diag(this, &c64Runner);
        diag.exec();
        updateUI();
    });

    QObject::connect(ui->actiondisable_joysticks, &QAction::triggered, [this](bool enabled) {
        std::cout << "actionoff:" << enabled << std::endl;
        if (enabled) {
            c64Runner.keyboard->setJoystick1Enabled(false);
            c64Runner.keyboard->setJoystick2Enabled(false);
            ui->actionjoystick_1->setChecked(false);
            ui->actionjoystick_2->setChecked(false);
            ui->actionboth_joysticks->setChecked(false);
            updateUI();
        }
    });
    QObject::connect(ui->actionjoystick_1, &QAction::triggered, [this](bool enabled) {
        if (enabled) {
            c64Runner.keyboard->setJoystick1Enabled(true);
            c64Runner.keyboard->setJoystick2Enabled(false);
            ui->actiondisable_joysticks->setChecked(false);
            ui->actionjoystick_2->setChecked(false);
            ui->actionboth_joysticks->setChecked(false);
            updateUI();
        }
    });
    QObject::connect(ui->actionjoystick_2, &QAction::triggered, [this](bool enabled) {
        if (enabled) {
            c64Runner.keyboard->setJoystick1Enabled(false);
            c64Runner.keyboard->setJoystick2Enabled(true);
            ui->actiondisable_joysticks->setChecked(false);
            ui->actionjoystick_1->setChecked(false);
            ui->actionboth_joysticks->setChecked(false);
            updateUI();
        }
    });
    QObject::connect(ui->actionboth_joysticks, &QAction::triggered, [this](bool enabled) {
        if (enabled) {
            c64Runner.keyboard->setJoystick1Enabled(true);
            c64Runner.keyboard->setJoystick2Enabled(true);
            ui->actiondisable_joysticks->setChecked(false);
            ui->actionjoystick_1->setChecked(false);
            ui->actionjoystick_2->setChecked(false);
            updateUI();
        }
    });

    frameTimer.setInterval(20); // 50Hz -> 20ms
    frameTimer.start();
    running = true;

    toolbarPauseAction = ui->toolBar->addAction("Pause", pauseUnpause);
    ui->toolBar->addAction("Step", step);
    ui->toolBar->addAction("Step Instr", stepInstruction);
    ui->toolBar->addAction("Step Line", stepLine);
    ui->toolBar->addAction("Step Frame", stepFrame);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction("Reset", hardReset);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction("MPU..", mpuViewer);
    ui->toolBar->addAction("RAM..", ramViewer);
    ui->toolBar->addAction("CIA..", ciaViewer);
    ui->toolBar->addAction("VIC..", vicViewer);
    ui->toolBar->addAction("Keyboard..", virtualKeyboard);
    ui->toolBar->addAction("Breaks..", breakpointEditor);

    auto tosize = [](const QPoint p) { return QSize { p.x(), p.y() }; };
    mainScreenOffset = size() - tosize(ui->mainScreenFrame->pos()) - ui->mainScreenFrame->size();

    mainScreen = new VideoWidget(ui->mainScreenFrame, VIC::screenWidth, VIC::screenHeight, &c64Runner.c64->vic.screen);
    ui->mainScreenFrame->layout()->addWidget(mainScreen);

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


void MainWindow::resizeEvent(QResizeEvent *event) {
    auto tosize = [](const QPoint p) { return QSize { p.x(), p.y() }; };
    ui->mainScreenFrame->resize(event->size() - mainScreenOffset - tosize(ui->mainScreenFrame->pos()));

    QMainWindow::resizeEvent(event);
}


void MainWindow::updateUI() {
    if (toolMPUViewer)
        toolMPUViewer->updateC64();
    if (toolRAMViewer)
        toolRAMViewer->updateC64();
    if (toolCIAViewer)
        toolCIAViewer->updateC64();
    if (toolVICViewer)
        toolVICViewer->updateC64();
    if (toolKeyboardWindow)
        toolKeyboardWindow->updateUI();
    if (toolJoystickWindow)
        toolJoystickWindow->updateUI();
    if (toolBreakpointEditor)
        toolBreakpointEditor->updateC64();

    mainScreen->setVideoBuffer(&c64Runner.c64->vic.screen);
    mainScreen->updateUI();

    std::stringstream ss;

    auto& mpu = c64Runner.c64->mpu;
    ss << "FPS:" << static_cast<int>(currentFPS) << " cycle:" << cycle << " frame:" << frame << " T:" << mpu.T;

    ss << " A:" << toHexStr(mpu.A);
    ss << " X:" << toHexStr(mpu.X);
    ss << " Y:" << toHexStr(mpu.Y);
    ss << " S:" << toHexStr(mpu.S);
    ss << " P:" << (mpu.P & 0x80 ? 'N' : '-') << (mpu.P & 0x40 ? 'V' : '-');
    ss << '-' << (mpu.P & 0x10 ? 'B' : '-') << (mpu.P & 0x08 ? 'D' : '-');
    ss << (mpu.P & 0x04 ? 'I' : '-') << (mpu.P & 0x02 ? 'Z' : '-') << (mpu.P & 0x01 ? 'C' : '-');
    ss << " PC:" << toHexStr(mpu.PC);

    ui->statusbar->showMessage(QString::fromStdString(ss.str()));

    // cheap screen
    /*
    std::cout << (char)0x1B << "[H+----------------------------------------+\n";
    for (int y = 0; y < 25; y++) {
        std::cout << '|';
        for (int x = 0; x < 40; x++) {
            char c = c64Runner.c64->mpuMemoryView.read(0x0400 + y * 40 + x, true);
            if (c >= 1 && c <= 26) std::cout << static_cast<char>(c - 1 + 'A');
            else if (c == 32) std::cout << ' ';
            else if (c == 40) std::cout << '(';
            else if (c == 41) std::cout << ')';
            else if (c == 46) std::cout << '.';
            else if (c >= 48 && c <= 57) std::cout << static_cast<char>(c - 48 + '0');
            else std::cout << '?';
        }
        std::cout << "|\n";
    }
    std::cout << "+----------------------------------------+\n";
    */
}
