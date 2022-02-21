#pragma once

#include <QDialog>

#include "../c64_runner.h"

namespace Ui {
class ExtractPRG;
}

class ExtractPRG : public QDialog
{
    Q_OBJECT

public:
    explicit ExtractPRG(QWidget *parent, C64Runner* c64Runner);
    ~ExtractPRG();

    void accept() override;

private:
    Ui::ExtractPRG *ui;
    C64Runner *c64Runner;
};

