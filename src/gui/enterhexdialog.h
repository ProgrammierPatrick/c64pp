#pragma once

#include <QDialog>

#include "c64_runner.h"

namespace Ui {
class EnterHexDialog;
}

class EnterHexDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnterHexDialog(QWidget *parent, C64Runner* c64Runner);
    ~EnterHexDialog();

    void accept() override;

private:
    Ui::EnterHexDialog *ui;
    C64Runner* c64Runner;
};

