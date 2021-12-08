#include "keyboard_matrix_widget.h"
#include "ui_keyboard_matrix_widget.h"

KeyboardMatrixWidget::KeyboardMatrixWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KeyboardMatrixWidget)
{
    ui->setupUi(this);
}

KeyboardMatrixWidget::~KeyboardMatrixWidget()
{
    delete ui;
}
