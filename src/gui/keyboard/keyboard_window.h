#pragma once

#include <QMainWindow>

#include "keyboard_widget.h"
#include "keyboard_matrix_widget.h"

namespace Ui {
class KeyboardWindow;
}

class MainWindow;

class KeyboardWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit KeyboardWindow(QWidget *parent, C64Runner *c64Runner, MainWindow *mainWindow);
    ~KeyboardWindow();

    void updateUI();

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    Ui::KeyboardWindow *ui;

    C64Runner *c64Runner;
    MainWindow *mainWindow;

    KeyboardWidget *keyboardWidget;
    KeyboardMatrixWidget *keyboardMatrix;

    bool showMatrix = false;
};

