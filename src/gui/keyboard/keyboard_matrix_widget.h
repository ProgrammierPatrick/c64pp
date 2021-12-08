#pragma once

#include <QWidget>

namespace Ui {
class KeyboardMatrixWidget;
}

class KeyboardMatrixWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KeyboardMatrixWidget(QWidget *parent = nullptr);
    ~KeyboardMatrixWidget();

private:
    Ui::KeyboardMatrixWidget *ui;
};

