#include "prg_loader.h"
#include "ui_prg_loader.h"
#include "text_utils.h"
#include "memory"

#include <QPushButton>
#include <QFileDialog>
#include <iostream>
#include <fstream>
#include <vector>

PRGLoader::PRGLoader(QWidget *parent, C64Runner* c64Runner, const std::string& fileName) :
    QDialog(parent),
    ui(new Ui::PRGLoader),
    c64Runner(c64Runner)
{
    ui->setupUi(this);

    std::ifstream file(fileName, std::ios::binary);
    file.unsetf(std::ios::skipws);

    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize == -1) {
        std::cout << "Error: file " << fileName << " could not be found." << std::endl;
    }
    auto dataPtr = std::make_shared<std::vector<uint8_t>>(std::istream_iterator<char>{file}, std::istream_iterator<char>{});
    auto& data = *dataPtr;
    file.close();
    std::cout << fileSize << " bytes loaded." << std::endl;

    // disable resizing
    setFixedSize(size());

    ui->targetNumber->setMaxLength(4);
    ui->targetNumber->setInputMask("HHHH");
    ui->offset->setMaxLength(4);
    ui->offset->setInputMask("HHHH");

    std::string startAddr = toHexStr(data[1]) + toHexStr(data[0]);

    ui->filePath->setText(QString::fromStdString(fileName));
    ui->offset->setText(QString::fromStdString(startAddr));

    uint16_t offset = data[0] | (data[1] << 8);
    auto rd8 = [&data,offset](uint16_t addr) {
        return data[addr - offset + 2];
    };
    auto rd16 = [&data,offset](uint16_t addr) {
        return static_cast<uint16_t>(data[addr - offset + 2] | (data[addr - offset + 3] << 8));
    };
    uint16_t nextLine = rd16(offset);
    bool singleLine = rd16(nextLine) == 0x0000;
    bool isSYS = data[6] == 0x9E;

    QPushButton *openAndRun = ui->buttonBox->addButton("Open And Run", QDialogButtonBox::AcceptRole);
    QPushButton *open = ui->buttonBox->addButton("Open", QDialogButtonBox::AcceptRole);

    if (startAddr == "0801" && singleLine && isSYS) {
        ui->fileTypeLabel->setText("Machine language with BASIC header");
        openAndRun->setDefault(true);
        uint16_t targetNum = 0;
        for (uint16_t i = 0x0806; i < nextLine - 1; i++) {
            if (rd8(i) == 0x20) {
                continue;
            }
            else if (rd8(i) >= 0x30 && rd8(i) <= 0x39) {
                targetNum = (targetNum * 10) + (rd8(i) & 0x0F);
            }
            else if (rd8(i) == 0x00) {
                return;
            }
        }
        ui->targetNumber->setText(QString::fromStdString(toHexStr(targetNum)));
    }
    else if (startAddr == "0801") {
        ui->fileTypeLabel->setText("BASIC");
        open->setDefault(true);
        ui->targetNumber->setText(QString::fromStdString(toHexStr(offset)));
    }
    else {
        ui->fileTypeLabel->setText("Generic");
        open->setDefault(true);
        ui->targetNumber->setText(QString::fromStdString(toHexStr(offset)));
    }

    QObject::connect(open, &QAbstractButton::clicked, [dataPtr, this](bool b){
        for (int i = 2; i < dataPtr->size(); i++) {
            this->c64Runner->c64->mpu.mem->write(fromHexStr16(ui->offset->text().toStdString()) + i - 2, (*dataPtr)[i]);
        }
    });


    QObject::connect(openAndRun, &QAbstractButton::clicked, [dataPtr, this](bool b){
        for (int i = 2; i < dataPtr->size(); i++) {
            this->c64Runner->c64->mpu.mem->write(fromHexStr16(ui->offset->text().toStdString()) + i - 2, (*dataPtr)[i]);
        }
        this->c64Runner->c64->mpu.PC = fromHexStr16(ui->targetNumber->text().toStdString());
        this->c64Runner->c64->mpu.T = 0;
    });


}
void PRGLoader::openPRGFile(QWidget *parent, C64Runner* c64Runner) {
    auto fileName = QFileDialog::getOpenFileName(parent, "Open PRG File","", "PRG File (*.prg);;All Files (*.*)").toStdString();
    if (fileName != "") {
        PRGLoader prgLoader(parent, c64Runner, fileName);
        prgLoader.exec();
    }
}

PRGLoader::~PRGLoader()
{
    delete ui;
}
