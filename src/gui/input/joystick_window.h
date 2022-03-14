#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>

#include "../c64_runner.h"

#include <vector>

namespace Ui {
class JoystickWindow;
}

class MainWindow;

class JoystickWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit JoystickWindow(MainWindow *parent, C64Runner* c64Runner);
    ~JoystickWindow();

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void updateUI();

private:
    void setJoyEnabled(bool joy1, bool enabled);

    MainWindow *mainWindow;
    Ui::JoystickWindow *ui;
    C64Runner *c64Runner;

    std::vector<QPushButton*> joy1Buttons;
    std::vector<QPushButton*> joy2Buttons;
    std::vector<QLabel*> helpLabels;
};

