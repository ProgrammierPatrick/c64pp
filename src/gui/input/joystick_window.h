#pragma once

#include <QMainWindow>

namespace Ui {
class JoystickWindow;
}

class JoystickWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit JoystickWindow(QWidget *parent = nullptr);
    ~JoystickWindow();

private:
    Ui::JoystickWindow *ui;
};

