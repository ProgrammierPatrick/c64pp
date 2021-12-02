#include "enterhexdialog.h"
#include "ui_enterhexdialog.h"

#include "text_utils.h"

#include <QMessageBox>

#include <vector>
#include <cstdint>
#include <iostream>

EnterHexDialog::EnterHexDialog(QWidget *parent, C64Runner *c64Runner) :
    QDialog(parent),
    c64Runner(c64Runner),
    ui(new Ui::EnterHexDialog)
{
    ui->setupUi(this);

    ui->offsetLineEdit->setMaxLength(4);
    ui->offsetLineEdit->setInputMask("HHHH");

    this->setWindowTitle("Enter Hex Data");
}

EnterHexDialog::~EnterHexDialog()
{
    delete ui;
}

void EnterHexDialog::accept() {
    auto offsetStr = ui->offsetLineEdit->text().toStdString();
    if (!isValidHex16(offsetStr)) {
        QMessageBox::critical(this, "Offset Error", QString::fromStdString("Offset value" + offsetStr + " is not a valid 16bit address. Do nothing."));
        return;
    }
    uint16_t offset = fromHexStr16(offsetStr);

    std::vector<uint8_t> data;
    bool first = true;
    std::string byteText = "00";
    for (char c : ui->plainTextEdit->toPlainText().toStdString()) {
        if (c == ' ' || c == '\t' || c == '\n')
            continue;
        if (first) {
            byteText[0] = c;
        } else {
            byteText[1] = c;
            if (!isValidHex8(byteText)) {
                QMessageBox::critical(this, "Syntax Error", QString::fromStdString("Encountered byte " + byteText + " while parsing. Do nothing."));
                return;
            }
            data.push_back(fromHexStr8(byteText));
        }
        first = !first;
    }

    if (ui->resetCheckBox->checkState())
        c64Runner->hardReset();

    for (uint16_t i = 0; i < data.size(); i++) {
        c64Runner->c64->mpuMemoryView.write(offset + i, data[i]);
    }

    std::cout << data.size() << " Bytes written to memory at offset " << toHexStr(offset) << std::endl;

    if (ui->setPCCheckBox) {
        c64Runner->c64->mpu.PCL = offset & 0xFF;
        c64Runner->c64->mpu.PCH = (offset >> 8) & 0xFF;
    }

    QDialog::accept();
}
