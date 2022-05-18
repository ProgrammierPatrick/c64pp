#include "main_window.h"
#include "ui_main_window.h"

#include "style.h"

#include "file/enterhexdialog.h"
#include "file/prg_loader.h"
#include "file/extract_prg.h"

#include "../emu/text_utils.h"

#include <QMediaDevices>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>

#include <iostream>
#include <sstream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icon-512.png"));

    titlebar = addDarkTitlebar(this);
    ui->menubar->resize(200, ui->menubar->height());

    frameTimer.setTimerType(Qt::TimerType::PreciseTimer);

    auto tickFPSStart = [this]() {
        auto t = std::chrono::system_clock::now();
        float newFPS = 1.f / std::chrono::duration<float>(t - lastFrameTime).count();
        currentFPS = 0.95f * currentFPS + 0.05f * newFPS; // exponential smoothing
        startTick = t;
        lastFrameTime = t;
    };
    auto tickFPSEnd = [this]() {
        auto stopTick = std::chrono::system_clock::now();
        float tickTime = std::chrono::duration<float>(stopTick - startTick).count();
        float newLoad = tickTime * currentFPS;
        currentLoad = 0.99f * currentLoad + 0.01f * newLoad; // exponential smoothing
    };

    auto hardReset = [this]() {
        c64Runner.hardReset();
        frame = 0;
        cycle = 0;
        setWindowTitle("C64++");
        updateUI();
    };

    auto start = [this]() {
        running = true;
        c64Runner.c64->breakPoints.resetBreakpoints();
        frameTimer.start();
        ui->pause->setIcon(QIcon(":/icons/pause.png"));
    };
    auto stop = [this]() {
        running = false;
        frameTimer.stop();
        ui->pause->setIcon(QIcon(":/icons/continue.png"));
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

    auto muteShortcut = [this]() {
        // mute
        if (!isMuted) {
            setVolume(0);
            ui->mute->setIcon(QIcon(":/icons/mute.png"));
            isMuted = true;
        }
        // unmute
        else if (isMuted) {
            setVolume(static_cast<double>(ui->volume->sliderPosition())/100);
            if (ui->volume->sliderPosition() == 0) ui->mute->setIcon(QIcon(":/icons/mute.png"));
            else if (ui->volume->sliderPosition() < 33) ui->mute->setIcon(QIcon(":/icons/low-vol.png"));
            else if (ui->volume->sliderPosition() < 67) ui->mute->setIcon(QIcon(":/icons/med-vol.png"));
            else ui->mute->setIcon(QIcon(":/icons/high-vol.png"));
            isMuted = false;
        }
    };

    // make shortcuts usable from other windows
    for (auto& menu : ui->menubar->findChildren<QMenu*>()) {
        for (auto& action : menu->actions()) {
            action->setShortcutContext(Qt::ShortcutContext::ApplicationShortcut);
        }
    }

    QObject::connect(ui->actionHard_Reset, &QAction::triggered, hardReset);
    QObject::connect(ui->actionPause, &QAction::triggered, pauseUnpause);
    QObject::connect(ui->actionStep, &QAction::triggered, step);
    QObject::connect(ui->actionStep_Instruction, &QAction::triggered, stepInstruction);
    QObject::connect(ui->actionStep_Line, &QAction::triggered, stepLine);
    QObject::connect(ui->actionStep_Frame, &QAction::triggered, stepFrame);

    QObject::connect(ui->actionMute, &QAction::triggered, muteShortcut);

    QObject::connect(ui->actionMPU_Viewer, &QAction::triggered, mpuViewer);
    QObject::connect(ui->actionRAM_Viewer, &QAction::triggered, ramViewer);
    QObject::connect(ui->actionCIA_Viewer, &QAction::triggered, ciaViewer);
    QObject::connect(ui->actionVIC_Viewer, &QAction::triggered, vicViewer);
    QObject::connect(ui->actionVirtual_Keyboard, &QAction::triggered, virtualKeyboard);
    QObject::connect(ui->actionVirtual_Joysticks, &QAction::triggered, joystickWindow);
    QObject::connect(ui->actionBreakpoint_Editor, &QAction::triggered, breakpointEditor);

    QObject::connect(&frameTimer, &QTimer::timeout, this, [this, stop, tickFPSStart, tickFPSEnd]() {
        try {
            tickFPSStart();
            cycle += c64Runner.stepFrame();
            c64Runner.c64->sid.process(audioBuffer.size(), audioBuffer.data());
            tickFPSEnd();

            audioOutputDevice->write(reinterpret_cast<const char*>(audioBuffer.data()), audioBuffer.size() * sizeof(float));
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

    ui->actiondisable_joysticks->trigger();
    QObject::connect(ui->actiondisable_joysticks, &QAction::triggered, [this](bool enabled) {
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

    ui->pause->setIconSize(QSize(17,17));
    ui->pause->setIcon(QIcon(":/icons/pause.png"));
    QObject::connect(ui->pause, &QPushButton::clicked, pauseUnpause);

    ui->reset->setIconSize(QSize(17,17));
    ui->reset->setIcon(QIcon(":/icons/reset.png"));
    QObject::connect(ui->reset, &QPushButton::clicked, hardReset);

    ui->keyboard->setIconSize(QSize(17,17));
    ui->keyboard->setIcon(QIcon(":/icons/keyboard.png"));
    QObject::connect(ui->keyboard, &QPushButton::clicked, virtualKeyboard);

    ui->joystick->setIconSize(QSize(17,17));
    ui->joystick->setIcon(QIcon(":/icons/joystick.png"));
    QObject::connect(ui->joystick, &QPushButton::clicked, joystickWindow);

    ui->step->setIconSize(QSize(17,17));
    ui->step->setIcon(QIcon(":/icons/step.png"));
    QObject::connect(ui->step, &QPushButton::clicked, step);

    ui->step_instruction->setIconSize(QSize(17,17));
    ui->step_instruction->setIcon(QIcon(":/icons/step_instruction.png"));
    QObject::connect(ui->step_instruction, &QPushButton::clicked, stepInstruction);

    ui->stepline->setIconSize(QSize(17,17));
    ui->stepline->setIcon(QIcon(":/icons/step_line.png"));
    QObject::connect(ui->stepline, &QPushButton::clicked, stepLine);

    ui->stepframe->setIconSize(QSize(17,17));
    ui->stepframe->setIcon(QIcon(":/icons/step_frame.png"));
    QObject::connect(ui->stepframe, &QPushButton::clicked, stepFrame);

    ui->breakpoints->setIconSize(QSize(17,17));
    ui->breakpoints->setIcon(QIcon(":/icons/breakpoints.png"));
    QObject::connect(ui->breakpoints, &QPushButton::clicked, breakpointEditor);

    ui->mpu_viewer->setIconSize(QSize(17,17));
    ui->mpu_viewer->setIcon(QIcon(":/icons/mpu_viewer.png"));
    QObject::connect(ui->mpu_viewer, &QPushButton::clicked, mpuViewer);

    ui->ram_viewer->setIconSize(QSize(17,17));
    ui->ram_viewer->setIcon(QIcon(":/icons/ram_viewer.png"));
    QObject::connect(ui->ram_viewer, &QPushButton::clicked, ramViewer);

    ui->cia_viewer->setIconSize(QSize(17,17));
    ui->cia_viewer->setIcon(QIcon(":/icons/cia_viewer.png"));
    QObject::connect(ui->cia_viewer, &QPushButton::clicked, ciaViewer);

    ui->vic_viewer->setIconSize(QSize(17,17));
    ui->vic_viewer->setIcon(QIcon(":/icons/vic_viewer.png"));
    QObject::connect(ui->vic_viewer, &QPushButton::clicked, vicViewer);

    auto tosize = [](const QPoint p) { return QSize { p.x(), p.y() }; };
    mainScreenOffset = size() - tosize(ui->mainScreenFrame->pos()) - ui->mainScreenFrame->size();
    framePos = ui->mainScreenFrame->pos();

    mainScreen = new VideoWidget(ui->mainScreenFrame, VIC::screenWidth, VIC::screenHeight, &c64Runner.c64->vic.screen);
    ui->mainScreenFrame->layout()->addWidget(mainScreen);
    ui->mainScreenFrame->setStyleSheet("QFrame { background-color: rgb(46,  51,  54); }");
    mainScreen->setStyleSheet("QFrame { background-color: rgb(46,  51,  54); }");

    ui->menubar->raise();

    QAudioFormat format;
    const int sampleRate = 44'000;
    format.setSampleRate(sampleRate);
    format.setChannelConfig(QAudioFormat::ChannelConfigMono);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Float);
    audioOutput = new QAudioSink(format, this);
    if (audioOutput->error() != QAudio::Error::NoError) {
        std::cout << "QAudioSink Error: " << audioOutput->error() << std::endl;
    }
    QObject::connect(audioOutput, &QAudioSink::stateChanged, [this](QAudio::State state) {
        // disable due to console spam
        // qDebug() << "state is now:" << state << "error:" << audioOutput->error() << "volume:" << audioOutput->volume();

        // fix linux audio issues
        if (audioOutput->error() == QAudio::Error::UnderrunError) {
            const float value[1] = {0};
            audioOutputDevice->write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
    });
    audioOutput->setBufferSize(10 * sizeof(float) * sampleRate / 50); // 200ms
    audioBuffer.resize(sampleRate / 50, 0);
    audioOutputDevice = audioOutput->start();
    if (!audioOutputDevice->isOpen()) {
        std::cout << "audio device not open!" << std::endl;
    }
    
    updateUI();

    ui->volume->setRange(0, 100);
    ui->volume->setSliderPosition(50);
    setVolume(static_cast<double>(ui->volume->sliderPosition())/100);
    if (ui->volume->sliderPosition() == 0) ui->mute->setIcon(QIcon(":/icons/mute.png"));
    else if (ui->volume->sliderPosition() < 33) ui->mute->setIcon(QIcon(":/icons/low-vol.png"));
    else if (ui->volume->sliderPosition() < 67) ui->mute->setIcon(QIcon(":/icons/med-vol.png"));
    else ui->mute->setIcon(QIcon(":/icons/high-vol.png"));
    ui->mute->setIconSize(QSize(17,17));

    // mute button is clicked
    QObject::connect(ui->mute, &QPushButton::clicked, [this] {
        // mute
        if (!isMuted) {
            setVolume(0);
            ui->mute->setIcon(QIcon(":/icons/mute.png"));
            isMuted = true;
        }
        // unmute
        else if (isMuted) {
            setVolume(static_cast<double>(ui->volume->sliderPosition())/100);
            if (ui->volume->sliderPosition() == 0) ui->mute->setIcon(QIcon(":/icons/mute.png"));
            else if (ui->volume->sliderPosition() < 33) ui->mute->setIcon(QIcon(":/icons/low-vol.png"));
            else if (ui->volume->sliderPosition() < 67) ui->mute->setIcon(QIcon(":/icons/med-vol.png"));
            else ui->mute->setIcon(QIcon(":/icons/high-vol.png"));
            isMuted = false;
        }
    });

    // slider is moved
    QObject::connect(ui->volume, &QSlider::valueChanged, [this] {
        // set icons depending on slider position
        if (ui->volume->sliderPosition() == 0) ui->mute->setIcon(QIcon(":/icons/mute.png"));
        else if (ui->volume->sliderPosition() < 33) ui->mute->setIcon(QIcon(":/icons/low-vol.png"));
        else if (ui->volume->sliderPosition() < 67) ui->mute->setIcon(QIcon(":/icons/med-vol.png"));
        else ui->mute->setIcon(QIcon(":/icons/high-vol.png"));

        setVolume(static_cast<double>(ui->volume->sliderPosition())/100);
    });

}

void MainWindow::setVolume(const double &vol) {
    audioOutput->setVolume(vol);
    updateUI();
}

void MainWindow::loadPRG(const std::string& prgPath) {
    PRGLoader prgLoader(this, &c64Runner, prgPath, true);
    prgLoader.show();
    prgLoader.close();
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
    QMainWindow::resizeEvent(event);

    auto tosize = [](const QPoint p) { return QSize { p.x(), p.y() }; };
    ui->mainScreenFrame->resize(event->size() - mainScreenOffset - tosize(ui->mainScreenFrame->pos()));
    for(auto& f : resizeCallbacks) f(event);

    ui->mainScreenFrame->move(framePos);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
    if (isFullScreen()) {
        ui->statusbar->show();
        ui->pause->show();
        ui->reset->show();
        ui->keyboard->show();
        ui->joystick->show();
        ui->mute->show();
        ui->volume->show();
        ui->step->show();
        ui->step_instruction->show();
        ui->stepframe->show();
        ui->stepline->show();
        ui->breakpoints->show();
        ui->mpu_viewer->show();
        ui->ram_viewer->show();
        ui->cia_viewer->show();
        ui->vic_viewer->show();
        showNormal();
        auto tosize = [](const QPoint p) { return QSize { p.x(), p.y() }; };
        auto topoint = [](const QSize p) { return QPoint { p.width(), p.height() }; };
        ui->mainScreenFrame->resize(windowSize - mainScreenOffset - tosize(ui->mainScreenFrame->pos()));
        ui->menubar->raise();
        titlebar->setHideButtons(false);
    }
    else {
        windowSize = this->size();
        ui->statusbar->hide();
        ui->pause->hide();
        ui->reset->hide();
        ui->keyboard->hide();
        ui->joystick->hide();
        ui->mute->hide();
        ui->volume->hide();
        ui->step->hide();
        ui->step_instruction->hide();
        ui->stepframe->hide();
        ui->stepline->hide();
        ui->breakpoints->hide();
        ui->mpu_viewer->hide();
        ui->ram_viewer->hide();
        ui->cia_viewer->hide();
        ui->vic_viewer->hide();
        showFullScreen();
        ui->mainScreenFrame->move(0,0);
        ui->mainScreenFrame->resize(size());
        ui->menubar->lower();
        titlebar->setHideButtons(true);
    }
    updateUI();
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
    ss << "FPS:" << currentFPS << "Load:" << static_cast<int>(100.0f * currentLoad) << "% cycle:" << cycle << " frame:" << frame << " T:" << mpu.T;

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
