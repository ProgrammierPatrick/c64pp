#pragma once

#include "c64_runner.h"

#include <QMainWindow>
#include <vector>

namespace Ui {
class CIAViewer;
}

class MainWindow;

class CIAViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit CIAViewer(MainWindow *parent, C64Runner *c64Runner);
    ~CIAViewer();

    void updateC64();

private:
    Ui::CIAViewer *ui;

    C64Runner *c64Runner;
    MainWindow *mainWindow;

    std::vector<uint8_t> charsetScreen;
};

