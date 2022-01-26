#pragma once

#include "c64_runner.h"
#include "mpu_viewer.h"
#include "cia_viewer.h"
#include "ram_viewer.h"
#include "breakpoint_editor.h"
#include "keyboard/keyboard_window.h"
#include "video/video_widget.h"

#include <QMainWindow>
#include <QTimer>

#include <vector>
#include <chrono>

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

private:
    Ui::MainWindow *ui;

    C64Runner c64Runner;
    long frame = 0;
    long cycle = 0;

    float running = true;
    QTimer frameTimer;
    std::chrono::time_point<std::chrono::system_clock> lastFrameTime;
    float currentFPS = 0;

    QAction* toolbarPauseAction;

    VideoWidget *mainScreen;

    MPUViewer *toolMPUViewer = nullptr;
    RAMViewer *toolRAMViewer = nullptr;
    CIAViewer *toolCIAViewer = nullptr;
    KeyboardWindow *toolKeyboardWindow = nullptr;
    BreakpointEditor *toolBreakpointEditor = nullptr;

    QSize mainScreenOffset;
};

