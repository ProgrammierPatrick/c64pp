#pragma once

#include "c64_runner.h"
#include "mpu_viewer.h"
#include "cia_viewer.h"
#include "ram_viewer.h"
#include "breakpoint_editor.h"
#include "input/keyboard_window.h"
#include "input/joystick_window.h"
#include "video/video_widget.h"
#include "video/vic_viewer.h"

#include <QMainWindow>
#include <QTimer>

#include <vector>
#include <chrono>

class QIODevice;

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

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;

    void registerResizeCallback(std::function<void(QResizeEvent*)> callback) { resizeCallbacks.push_back(callback); }

    void tickFrames(int frameCount) {
        for(int i = 0; i < frameCount; i++)
            c64Runner.stepFrame();
        updateUI();
    }
    void loadPRG(const std::string& prgPath);

    void setVolume(const double& vol);

private:
    Ui::MainWindow *ui;

    C64Runner c64Runner;
    long frame = 0;
    long cycle = 0;

    float running = true;
    QTimer frameTimer;
    std::chrono::time_point<std::chrono::system_clock> lastFrameTime;
    std::chrono::time_point<std::chrono::system_clock> startTick;
    float currentFPS = 0;
    float currentLoad = 0;

    QAction* toolbarPauseAction;

    VideoWidget *mainScreen;

    MPUViewer *toolMPUViewer = nullptr;
    RAMViewer *toolRAMViewer = nullptr;
    CIAViewer *toolCIAViewer = nullptr;
    VICViewer *toolVICViewer = nullptr;
    KeyboardWindow *toolKeyboardWindow = nullptr;
    JoystickWindow *toolJoystickWindow = nullptr;
    BreakpointEditor *toolBreakpointEditor = nullptr;

    QSize mainScreenOffset;

    QIODevice *audioOutputDevice;
    std::vector<float> audioBuffer;

    bool isMuted = false;
    int volumeIntensity = 100; // sets the inital volume of the system (range 0 - 100)

    std::vector<std::function<void(QResizeEvent*)>> resizeCallbacks;
};

