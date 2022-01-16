#pragma once

#include "c64_runner.h"
#include "mpu_viewer.h"
#include "cia_viewer.h"
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

    VideoWidget *mainScreen;

    MPUViewer *toolMPUViewer = nullptr;
    CIAViewer *toolCIAViewer = nullptr;
    KeyboardWindow *toolKeyboardWindow = nullptr;

    QSize mainScreenOffset;
};

