#pragma once

#include "keyboard_state.h"
#include "../c64_runner.h"

#include <QWidget>
#include <QPushButton>

#include <vector>
#include <tuple>

namespace Ui {
class KeyboardWidget;
}

class MainWindow;

class KeyboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KeyboardWidget(QWidget *parent, C64Runner* c64Runner, MainWindow *mainWindow);
    ~KeyboardWidget();

    void updateUI();

    void showBindings();
    void hideBindings();
    void toggleBindings();
    void updateBindings();

private:
    void initVirtualKeyboard();

    Ui::KeyboardWidget *ui;
    MainWindow *mainWindow;
    C64Runner *c64Runner;

    bool bindingsShown = false;

    std::vector<std::tuple<QPushButton*,int,int>> virtualKeyboard;
};

