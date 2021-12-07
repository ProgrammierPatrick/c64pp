#include "mpu_viewer.h"
#include "ui_mpu_viewer.h"

#include "main_window.h"
#include "text_utils.h"

#include <iostream>

MPUViewer::MPUViewer(MainWindow *parent, C64Runner *c64Runner) :
    QMainWindow(parent),
    ui(new Ui::MPUViewer),
    mainWindow(parent),
    c64Runner(c64Runner)
{
    ui->setupUi(this);

    // disable resizing
    setFixedSize(size());

    auto setupLineEdit8 = [](QLineEdit* edit) {
        edit->setMaxLength(2);
        edit->setInputMask("HH");
    };
    setupLineEdit8(ui->lineEditA);
    setupLineEdit8(ui->lineEditX);
    setupLineEdit8(ui->lineEditY);
    setupLineEdit8(ui->lineEditS);
    ui->lineEditPC->setMaxLength(4);
    ui->lineEditPC->setInputMask("HHHH");

    QObject::connect(ui->lineEditA, &QLineEdit::editingFinished, [this]() {
        auto text = ui->lineEditA->text().toStdString();
        if (isValidHex8(text)) {
            auto& mpu = this->c64Runner->c64->mpu;
            mpu.A = fromHexStr8(text);
            ui->lineEditA->setText(QString::fromStdString(toHexStr(mpu.A)));
            mainWindow->updateUI();
        }
    });
    QObject::connect(ui->lineEditX, &QLineEdit::editingFinished, [this]() {
        auto text = ui->lineEditX->text().toStdString();
        if (isValidHex8(text)) {
            auto& mpu = this->c64Runner->c64->mpu;
            mpu.X = fromHexStr8(text);
            ui->lineEditX->setText(QString::fromStdString(toHexStr(mpu.X)));
            mainWindow->updateUI();
        }
    });
    QObject::connect(ui->lineEditY, &QLineEdit::editingFinished, [this]() {
        auto text = ui->lineEditY->text().toStdString();
        if (isValidHex8(text)) {
            auto& mpu = this->c64Runner->c64->mpu;
            mpu.Y = fromHexStr8(text);
            ui->lineEditY->setText(QString::fromStdString(toHexStr(mpu.Y)));
            mainWindow->updateUI();
        }
    });
    QObject::connect(ui->lineEditS, &QLineEdit::editingFinished, [this]() {
        auto text = ui->lineEditS->text().toStdString();
        if (isValidHex8(text)) {
            auto& mpu = this->c64Runner->c64->mpu;
            mpu.S = fromHexStr8(text);
            ui->lineEditS->setText(QString::fromStdString(toHexStr(mpu.S)));
            mainWindow->updateUI();
        }
    });
    QObject::connect(ui->lineEditPC, &QLineEdit::editingFinished, [this]() {
        auto text = ui->lineEditPC->text().toStdString();
        if (isValidHex16(text)) {
            auto& mpu = this->c64Runner->c64->mpu;
            mpu.PC = fromHexStr16(text);
            ui->lineEditPC->setText(QString::fromStdString(toHexStr(mpu.PC)));
            mainWindow->updateUI();
        }
    });

    QObject::connect(ui->checkboxC, &QCheckBox::stateChanged, [this]() {
        auto& checkbox = ui->checkboxC;
        uint8_t flag = MPU::Flag::C;
        if (checkbox->isChecked()) this->c64Runner->c64->mpu.P |= flag;
        else                        this->c64Runner->c64->mpu.P &= ~flag;
        mainWindow->updateUI();
    });
    QObject::connect(ui->checkboxZ, &QCheckBox::stateChanged, [this]() {
        auto& checkbox = ui->checkboxZ;
        uint8_t flag = MPU::Flag::Z;
        if (checkbox->isChecked()) this->c64Runner->c64->mpu.P |= flag;
        else                        this->c64Runner->c64->mpu.P &= ~flag;
        mainWindow->updateUI();
    });
    QObject::connect(ui->checkboxI, &QCheckBox::stateChanged, [this]() {
        auto& checkbox = ui->checkboxI;
        uint8_t flag = MPU::Flag::I;
        if (checkbox->isChecked()) this->c64Runner->c64->mpu.P |= flag;
        else                        this->c64Runner->c64->mpu.P &= ~flag;
        mainWindow->updateUI();
    });
    QObject::connect(ui->checkboxD, &QCheckBox::stateChanged, [this]() {
        auto& checkbox = ui->checkboxD;
        uint8_t flag = MPU::Flag::D;
        if (checkbox->isChecked()) this->c64Runner->c64->mpu.P |= flag;
        else                        this->c64Runner->c64->mpu.P &= ~flag;
        mainWindow->updateUI();
    });
    QObject::connect(ui->checkboxB, &QCheckBox::stateChanged, [this]() {
        auto& checkbox = ui->checkboxB;
        uint8_t flag = MPU::Flag::B;
        if (checkbox->isChecked()) this->c64Runner->c64->mpu.P |= flag;
        else                        this->c64Runner->c64->mpu.P &= ~flag;
        mainWindow->updateUI();
    });
    QObject::connect(ui->checkboxV, &QCheckBox::stateChanged, [this]() {
        auto& checkbox = ui->checkboxV;
        uint8_t flag = MPU::Flag::V;
        if (checkbox->isChecked()) this->c64Runner->c64->mpu.P |= flag;
        else                        this->c64Runner->c64->mpu.P &= ~flag;
        mainWindow->updateUI();
    });
    QObject::connect(ui->checkboxN, &QCheckBox::stateChanged, [this]() {
        auto& checkbox = ui->checkboxN;
        uint8_t flag = MPU::Flag::N;
        if (checkbox->isChecked()) this->c64Runner->c64->mpu.P |= flag;
        else                        this->c64Runner->c64->mpu.P &= ~flag;
        mainWindow->updateUI();
    });

    updateC64();
}

MPUViewer::~MPUViewer()
{
    delete ui;
}

void MPUViewer::updateC64() {
    auto& mpu = c64Runner->c64->mpu;
    auto setText = [](QLineEdit& edit, const std::string& text) {
        if (edit.text().toStdString() != text && !edit.isModified()) {
            auto cursorPos = edit.cursorPosition();
            edit.setText(QString::fromStdString(text));
            edit.setCursorPosition(cursorPos);
        }
    };

    setText(*ui->lineEditA, toHexStr(mpu.A));
    setText(*ui->lineEditX, toHexStr(mpu.X));
    setText(*ui->lineEditY, toHexStr(mpu.Y));
    setText(*ui->lineEditS, toHexStr(mpu.S));
    setText(*ui->lineEditPC, toHexStr(mpu.PC));

    if (ui->checkboxC->isChecked() != (mpu.P & MPU::Flag::C)) ui->checkboxC->setChecked(mpu.P & MPU::Flag::C);
    if (ui->checkboxZ->isChecked() != (mpu.P & MPU::Flag::Z)) ui->checkboxC->setChecked(mpu.P & MPU::Flag::Z);
    if (ui->checkboxI->isChecked() != (mpu.P & MPU::Flag::I)) ui->checkboxC->setChecked(mpu.P & MPU::Flag::I);
    if (ui->checkboxD->isChecked() != (mpu.P & MPU::Flag::D)) ui->checkboxC->setChecked(mpu.P & MPU::Flag::D);
    if (ui->checkboxB->isChecked() != (mpu.P & MPU::Flag::B)) ui->checkboxC->setChecked(mpu.P & MPU::Flag::B);
    if (ui->checkboxV->isChecked() != (mpu.P & MPU::Flag::V)) ui->checkboxC->setChecked(mpu.P & MPU::Flag::V);
    if (ui->checkboxN->isChecked() != (mpu.P & MPU::Flag::N)) ui->checkboxC->setChecked(mpu.P & MPU::Flag::N);
}
