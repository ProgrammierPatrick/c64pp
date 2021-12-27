#pragma once

#include "c64_runner.h"
#include "mpu_viewer.h"
#include "keyboard/keyboard_window.h"
#include "video/video_widget.h"

#include <QMainWindow>
#include <QTimer>

#include <vector>

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

    QAction* toolbarPauseAction;

    std::vector<uint8_t> mainScreenBuffer = std::vector<uint8_t>(40 * 16 * 25 * 16);
    VideoWidget *mainScreen;

    MPUViewer *toolMPUViewer = nullptr;
    KeyboardWindow *toolKeyboardWindow = nullptr;

    QSize mainScreenOffset;
};

