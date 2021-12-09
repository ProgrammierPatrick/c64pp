#pragma once

#include "c64_runner.h"
#include "mpu_viewer.h"
#include "keyboard/keyboard_widget.h"
#include "keyboard/keyboard_window.h"

#include <QMainWindow>
#include <QTimer>

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

private:
    Ui::MainWindow *ui;

    C64Runner c64Runner;
    long frame = 0;
    long cycle = 0;

    float running = true;
    QTimer frameTimer;

    QAction* toolbarPauseAction;

    // TODO: remove if proper keyboard window, only temporary
    KeyboardWidget *keyboardWidget;

    MPUViewer *toolMPUViewer = nullptr;
    KeyboardWindow *toolKeyboardWindow = nullptr;
};

