#pragma once

#include <QMainWindow>

#include "c64_runner.h"

namespace Ui {
class BreakpointEditor;
}

class MainWindow;

class BreakpointEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit BreakpointEditor(MainWindow *parent, C64Runner *c64Runner);
    ~BreakpointEditor();
    void updateC64();

private:
    Ui::BreakpointEditor *ui;
    C64Runner *c64Runner;
    MainWindow *mainWindow;
};
