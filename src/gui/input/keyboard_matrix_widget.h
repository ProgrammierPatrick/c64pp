#pragma once

#include <QWidget>

#include "../c64_runner.h"

namespace Ui {
class KeyboardMatrixWidget;
}

class KeyboardMatrixWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KeyboardMatrixWidget(QWidget *parent, C64Runner *c64Runner);
    ~KeyboardMatrixWidget();

    void updateUI();

private:
    Ui::KeyboardMatrixWidget *ui;
    C64Runner* c64Runner;
};

