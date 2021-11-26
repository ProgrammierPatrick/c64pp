#pragma once

#include "c64_runner.h"

#include <QMainWindow>

namespace Ui {
class MPUViewer;
}

class MPUViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit MPUViewer(QWidget *parent, C64Runner* c64Runner);
    ~MPUViewer();

    void updateC64();

private:
    Ui::MPUViewer *ui;
    C64Runner *c64Runner;
};

