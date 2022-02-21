#include "extract_prg.h"
#include "ui_extract_prg.h"

#include "../text_utils.h"

#include <QFileDialog>
#include <fstream>

ExtractPRG::ExtractPRG(QWidget *parent, C64Runner *c64Runner) :
    QDialog(parent),
    c64Runner(c64Runner),
    ui(new Ui::ExtractPRG)
{
    ui->setupUi(this);

    ui->firstAddr->setMaxLength(4);
    ui->firstAddr->setInputMask("HHHH");
    ui->lastAddr->setMaxLength(4);
    ui->lastAddr->setInputMask("HHHH");

    this->setWindowTitle("Extract RAM to file");
    this->setFixedSize(size());
}

ExtractPRG::~ExtractPRG()
{
    delete ui;
}

void ExtractPRG::accept()
{
    uint16_t firstAddr = fromHexStr16(ui->firstAddr->text().toStdString());
    uint16_t lastAddr = fromHexStr16(ui->lastAddr->text().toStdString());

    switch(ui->modeComboBox->currentIndex()) {
    case 0: { // PRG with offset
        auto filename = QFileDialog::getSaveFileName(this, "save to PRG file with offset", "", "PRG File (*.prg);;*").toStdString();
        if (filename == "") return;
        std::ofstream file(filename, std::ios::out | std::ios::binary);
        if (!file) throw std::runtime_error("Cannot open file '" + filename + "' for writing.");
        file.unsetf(std::ios::skipws);
        file << static_cast<uint8_t>(firstAddr);
        file << static_cast<uint8_t>(firstAddr >> 8);
        for(uint16_t addr = firstAddr; addr <= lastAddr; addr++) {
            file << c64Runner->c64->mainRAM.read(addr, true);
        }
        file.close();
    } break;
    case 1: { // raw data
        auto filename = QFileDialog::getSaveFileName(this, "save to raw file", "", "BIN file (*.bin);;*").toStdString();
        if (filename == "") return;
        std::ofstream file(filename, std::ios::out | std::ios::binary);
        if (!file) throw std::runtime_error("Cannot open file '" + filename + "' for writing.");
        file.unsetf(std::ios::skipws);
        for(uint16_t addr = firstAddr; addr <= lastAddr; addr++) {
            file << c64Runner->c64->mainRAM.read(addr, true);
        }
        file.close();
    } break;
    case 2: { // hex data
        auto filename = QFileDialog::getSaveFileName(this, "save to hex file", "", "HEX File (*.hex);;TXT File (*.txt);;*").toStdString();
        if (filename == "") return;
        std::ofstream file(filename, std::ios::out);
        if (!file) throw std::runtime_error("Cannot open file '" + filename + "' for writing.");
        for (int i = 0; i < firstAddr % 16; i++)
            file << "   ";
        for(uint16_t addr = firstAddr; addr <= lastAddr; addr++) {
            file << toHexStr(c64Runner->c64->mainRAM.read(addr, true));
            file << ((addr % 16 == 15 || addr == lastAddr) ? "\n" : " ");
        }
        file.close();
    } break;
    default: throw std::runtime_error("ExtractPRG::accept(): unknown ComboBox index " + std::to_string(ui->modeComboBox->currentIndex()));
    }

    QDialog::accept();
}
