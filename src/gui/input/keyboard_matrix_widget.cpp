#include "keyboard_matrix_widget.h"
#include "ui_keyboard_matrix_widget.h"

KeyboardMatrixWidget::KeyboardMatrixWidget(QWidget *parent, C64Runner *c64Runner) :
    QWidget(parent),
    ui(new Ui::KeyboardMatrixWidget),
    c64Runner(c64Runner)
{
    ui->setupUi(this);

    for(int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            ui->tableWidget->setItem(row, col, new QTableWidgetItem(" "));
        }
    }
}

KeyboardMatrixWidget::~KeyboardMatrixWidget()
{
    delete ui;
}


void KeyboardMatrixWidget::updateUI() {
    for(int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            QString s = c64Runner->keyboard->querySingleKey(row, col) ? "X":" ";
            ui->tableWidget->item(row, col)->setText(s);
        }
    }
}
