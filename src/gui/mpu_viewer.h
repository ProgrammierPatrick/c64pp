#pragma once

#include "c64_runner.h"

#include <QMainWindow>

namespace Ui {
class MPUViewer;
}

class MainWindow;

class MPUViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit MPUViewer(MainWindow *parent, C64Runner* c64Runner);
    ~MPUViewer();

    void updateC64();

private:
    Ui::MPUViewer *ui;
    C64Runner *c64Runner;
    MainWindow *mainWindow;
};

