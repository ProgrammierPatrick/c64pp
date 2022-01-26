#pragma once

#include <QMainWindow>

#include "c64_runner.h"

namespace Ui {
class BreakpointEditor;
}

class BreakpointEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit BreakpointEditor(QWidget *parent, C64Runner *c64Runner);
    ~BreakpointEditor();
    void updateC64();

private:
    Ui::BreakpointEditor *ui;
    C64Runner *c64Runner;
};
