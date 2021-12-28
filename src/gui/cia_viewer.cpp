#include "cia_viewer.h"
#include "ui_cia_viewer.h"

#include "main_window.h"
#include "../emu/io/cia.h"

#include "text_utils.h"

#include <cstdint>

CIAViewer::CIAViewer(MainWindow *parent, C64Runner *c64Runner) :
    QMainWindow(parent),
    ui(new Ui::CIAViewer),
    mainWindow(parent),
    c64Runner(c64Runner)
{
    ui->setupUi(this);

    setFixedSize(size());

    // setup line edit input masks
    auto setupLineEdit8  = [](QLineEdit* edit) { edit->setMaxLength(2); edit->setInputMask("HH"); };
    auto setupLineEdit16 = [](QLineEdit* edit) { edit->setMaxLength(4); edit->setInputMask("HHHH"); };
    setupLineEdit8(ui->CIA1PRA);
    setupLineEdit8(ui->CIA1PRB);
    setupLineEdit16(ui->CIA1TAread);
    setupLineEdit16(ui->CIA1TAwrite);
    setupLineEdit16(ui->CIA1TBread);
    setupLineEdit16(ui->CIA1TBwrite);
    setupLineEdit16(ui->CIA2TAread);
    setupLineEdit16(ui->CIA2TAwrite);
    setupLineEdit16(ui->CIA2TBread);
    setupLineEdit16(ui->CIA2TBwrite);
    setupLineEdit8(ui->CIA1ICRMask);
    setupLineEdit8(ui->CIA1ICRData);
    setupLineEdit8(ui->CIA2ICRMask);
    setupLineEdit8(ui->CIA2ICRData);
    setupLineEdit8(ui->CIA1CRA);
    setupLineEdit8(ui->CIA1CRB);
    setupLineEdit8(ui->CIA2CRA);
    setupLineEdit8(ui->CIA2CRB);
    setupLineEdit16(ui->timer1ALatch);
    setupLineEdit16(ui->timer1BLatch);
    setupLineEdit16(ui->timer2ALatch);
    setupLineEdit16(ui->timer2BLatch);
    setupLineEdit16(ui->timer1ACounter);
    setupLineEdit16(ui->timer1BCounter);
    setupLineEdit16(ui->timer2ACounter);
    setupLineEdit16(ui->timer2BCounter);

    // connect callbacks
    auto connect8 = [this](QLineEdit* edit, std::function<void(CIA&,uint8_t)> func) {
        QObject::connect(edit, &QLineEdit::editingFinished, [edit,func,this]() {
            auto text = edit->text().toStdString();
            if (isValidHex8(text)) {
                func(this->c64Runner->c64->cia, fromHexStr8(text));
                edit->setModified(false);
                mainWindow->updateUI();
            }
        });
    };
    auto connect16 = [this](QLineEdit* edit, std::function<void(CIA&,uint16_t)> func) {
        QObject::connect(edit, &QLineEdit::editingFinished, [edit,func,this]() {
            auto text = edit->text().toStdString();
            if (isValidHex16(text)) {
                func(this->c64Runner->c64->cia, fromHexStr16(text));
                edit->setModified(false);
                mainWindow->updateUI();
            }
        });
    };
    auto connectCombo = [this](QComboBox* box, std::function<void(CIA&, int)> func) {
        QObject::connect(box, &QComboBox::activated, [func,this](int index) {
            func(this->c64Runner->c64->cia, index);
            mainWindow->updateUI();
        });
    };
    auto connectButton = [this](QPushButton* button, std::function<void(CIA&)> func) {
        QObject::connect(button, &QPushButton::clicked, [button,func,this](int index) {
            func(this->c64Runner->c64->cia);
            mainWindow->updateUI();
        });
    };
    auto connectCheckbox = [this](QCheckBox* box, std::function<void(CIA&,bool)> func) {
        QObject::connect(box, &QCheckBox::stateChanged, [box,func,this]() {
            func(this->c64Runner->c64->cia, box->isChecked());
            mainWindow->updateUI();
        });
    };
    connect8(ui->CIA1PRA, [](CIA& cia, uint8_t val) { cia.write(0x0000, val); });
    connect16(ui->CIA1TAread , [](CIA& cia, uint16_t val) { cia.timerCIA1.counterA = val; });
    connect16(ui->CIA1TAwrite, [](CIA& cia, uint16_t val) { cia.timerCIA1.latchA = val; });
    connect16(ui->CIA1TBread , [](CIA& cia, uint16_t val) { cia.timerCIA1.counterB = val; });
    connect16(ui->CIA1TBwrite, [](CIA& cia, uint16_t val) { cia.timerCIA1.latchB = val; });
    connect16(ui->CIA2TAread , [](CIA& cia, uint16_t val) { cia.timerCIA2.counterA = val; });
    connect16(ui->CIA2TAwrite, [](CIA& cia, uint16_t val) { cia.timerCIA2.latchA = val; });
    connect16(ui->CIA2TBread , [](CIA& cia, uint16_t val) { cia.timerCIA2.counterB = val; });
    connect16(ui->CIA2TBwrite, [](CIA& cia, uint16_t val) { cia.timerCIA2.latchB = val; });
    connect8(ui->CIA1ICRMask, [](CIA& cia, uint8_t val) { cia.ICRMask1 = val; });
    connect8(ui->CIA2ICRMask, [](CIA& cia, uint8_t val) { cia.ICRMask2 = val; });
    connect8(ui->CIA1ICRData, [](CIA& cia, uint8_t val) { cia.ICRData1 = val; });
    connect8(ui->CIA2ICRData, [](CIA& cia, uint8_t val) { cia.ICRData2 = val; });
    connect8(ui->CIA1CRA, [](CIA& cia, uint8_t val) { cia.timerCIA1.writeCRA(val); });
    connect8(ui->CIA1CRB, [](CIA& cia, uint8_t val) { cia.timerCIA1.writeCRB(val); });
    connect8(ui->CIA2CRA, [](CIA& cia, uint8_t val) { cia.timerCIA2.writeCRA(val); });
    connect8(ui->CIA2CRB, [](CIA& cia, uint8_t val) { cia.timerCIA2.writeCRB(val); });
    connect16(ui->timer1ALatch, [](CIA& cia, uint16_t val) { cia.timerCIA1.latchA = val; });
    connect16(ui->timer1BLatch, [](CIA& cia, uint16_t val) { cia.timerCIA1.latchB = val; });
    connect16(ui->timer2ALatch, [](CIA& cia, uint16_t val) { cia.timerCIA2.latchA = val; });
    connect16(ui->timer2BLatch, [](CIA& cia, uint16_t val) { cia.timerCIA2.latchB = val; });
    connect16(ui->timer1ACounter, [](CIA& cia, uint16_t val) { cia.timerCIA1.counterA = val; });
    connect16(ui->timer1BCounter, [](CIA& cia, uint16_t val) { cia.timerCIA1.counterB = val; });
    connect16(ui->timer2ACounter, [](CIA& cia, uint16_t val) { cia.timerCIA2.counterA = val; });
    connect16(ui->timer2BCounter, [](CIA& cia, uint16_t val) { cia.timerCIA2.counterB = val; });
    connectCombo(ui->CR1A5, [](CIA& cia, int val) { cia.timerCIA1.writeCRA(cia.timerCIA1.readCRA() & ~0x20 | (val << 5)); });
    connectCombo(ui->CR1B5, [](CIA& cia, int val) { cia.timerCIA1.writeCRB(cia.timerCIA1.readCRB() & ~0x60 | (val << 5)); });
    connectCombo(ui->CR2A5, [](CIA& cia, int val) { cia.timerCIA2.writeCRA(cia.timerCIA2.readCRA() & ~0x20 | (val << 5)); });
    connectCombo(ui->CR2B5, [](CIA& cia, int val) { cia.timerCIA2.writeCRB(cia.timerCIA2.readCRB() & ~0x60 | (val << 5)); });
    connectButton(ui->CR1ALoad, [](CIA& cia) { cia.timerCIA1.writeCRA(cia.timerCIA1.readCRA() | 0x10); });
    connectButton(ui->CR1BLoad, [](CIA& cia) { cia.timerCIA1.writeCRB(cia.timerCIA1.readCRB() | 0x10); });
    connectButton(ui->CR2ALoad, [](CIA& cia) { cia.timerCIA2.writeCRA(cia.timerCIA2.readCRA() | 0x10); });
    connectButton(ui->CR2BLoad, [](CIA& cia) { cia.timerCIA2.writeCRB(cia.timerCIA2.readCRB() | 0x10); });
    connectCombo(ui->CR1A3, [](CIA& cia, int val) { cia.timerCIA1.writeCRA(cia.timerCIA1.readCRA() & ~0x08 | (val << 3)); });
    connectCombo(ui->CR1B3, [](CIA& cia, int val) { cia.timerCIA1.writeCRB(cia.timerCIA1.readCRB() & ~0x08 | (val << 3)); });
    connectCombo(ui->CR2A3, [](CIA& cia, int val) { cia.timerCIA2.writeCRA(cia.timerCIA2.readCRA() & ~0x08 | (val << 3)); });
    connectCombo(ui->CR2B3, [](CIA& cia, int val) { cia.timerCIA2.writeCRB(cia.timerCIA2.readCRB() & ~0x08 | (val << 3)); });
    connectCheckbox(ui->CR1A0, [](CIA& cia, bool val) { cia.timerCIA1.writeCRA(cia.timerCIA1.readCRA() & ~0x01 | (val ? 0x01 : 0)); });
    connectCheckbox(ui->CR1B0, [](CIA& cia, bool val) { cia.timerCIA1.writeCRB(cia.timerCIA1.readCRB() & ~0x01 | (val ? 0x01 : 0)); });
    connectCheckbox(ui->CR2A0, [](CIA& cia, bool val) { cia.timerCIA2.writeCRA(cia.timerCIA2.readCRA() & ~0x01 | (val ? 0x01 : 0)); });
    connectCheckbox(ui->CR2B0, [](CIA& cia, bool val) { cia.timerCIA2.writeCRB(cia.timerCIA2.readCRB() & ~0x01 | (val ? 0x01 : 0)); });
    connectCheckbox(ui->IRQ0Data, [](CIA& cia, bool val) {
        cia.ICRData1 = cia.ICRData1 & ~0x01 | (val ? 0x01 : 0x00);
        cia.IRQ = cia.ICRData1 & 0x7F;
        cia.ICRData1 = cia.ICRData1 & ~0x80 | (cia.IRQ ? 0x80 : 0x00);
    });
    connectCheckbox(ui->IRQ1Data, [](CIA& cia, bool val) {
        cia.ICRData1 = cia.ICRData1 & ~0x02 | (val ? 0x02 : 0x00);
        cia.IRQ = cia.ICRData1 & 0x7F;
        cia.ICRData1 = cia.ICRData1 & ~0x80 | (cia.IRQ ? 0x80 : 0x00);
    });
    connectCheckbox(ui->NMI0Data, [](CIA& cia, bool val) {
        cia.ICRData2 = cia.ICRData2 & ~0x01 | (val ? 0x01 : 0x00);
        cia.NMI = cia.ICRData2 & 0x7F;
        cia.ICRData2 = cia.ICRData2 & ~0x80 | (cia.NMI ? 0x80 : 0x00);
    });
    connectCheckbox(ui->NMI1Data, [](CIA& cia, bool val) {
        cia.ICRData2 = cia.ICRData2 & ~0x02 | (val ? 0x02 : 0x00);
        cia.NMI = cia.ICRData2 & 0x7F;
        cia.ICRData2 = cia.ICRData2 & ~0x80 | (cia.NMI ? 0x80 : 0x00);
    });
    connectCheckbox(ui->IRQ0Mask, [](CIA& cia, bool val) { cia.ICRMask1 = cia.ICRMask1 & ~0x01 | (val ? 0x01 : 0x00); });
    connectCheckbox(ui->IRQ1Mask, [](CIA& cia, bool val) { cia.ICRMask1 = cia.ICRMask1 & ~0x02 | (val ? 0x02 : 0x00); });
    connectCheckbox(ui->NMI0Mask, [](CIA& cia, bool val) { cia.ICRMask2 = cia.ICRMask2 & ~0x01 | (val ? 0x01 : 0x00); });
    connectCheckbox(ui->NMI1Mask, [](CIA& cia, bool val) { cia.ICRMask2 = cia.ICRMask2 & ~0x02 | (val ? 0x02 : 0x00); });

    updateC64();
}

CIAViewer::~CIAViewer()
{
    delete ui;
}

void CIAViewer::updateC64() {
    auto& cia = c64Runner->c64->cia;

    auto setText = [](QLineEdit* edit, const std::string& text) {
        if (edit->text().toStdString() != text && !edit->isModified()) {
            auto cursorPos = edit->cursorPosition();
            edit->setText(QString::fromStdString(text));
            edit->setCursorPosition(cursorPos);
        }
    };

    setText(ui->CIA1PRA, toHexStr(cia.read(0x0000)));
    setText(ui->CIA1PRB, toHexStr(cia.read(0x0001)));
    setText(ui->CIA1TAread, toHexStr(cia.timerCIA1.counterA));
    setText(ui->CIA1TAwrite, toHexStr(cia.timerCIA1.latchA));
    setText(ui->CIA1TBread, toHexStr(cia.timerCIA1.counterB));
    setText(ui->CIA1TBwrite, toHexStr(cia.timerCIA1.latchB));
    setText(ui->CIA2TAread, toHexStr(cia.timerCIA2.counterA));
    setText(ui->CIA2TAwrite, toHexStr(cia.timerCIA2.latchA));
    setText(ui->CIA2TBread, toHexStr(cia.timerCIA2.counterB));
    setText(ui->CIA2TBwrite, toHexStr(cia.timerCIA2.latchB));
    setText(ui->CIA1ICRMask, toHexStr(cia.ICRMask1));
    setText(ui->CIA2ICRMask, toHexStr(cia.ICRMask2));
    setText(ui->CIA1ICRData, toHexStr(cia.ICRData1));
    setText(ui->CIA2ICRData, toHexStr(cia.ICRData2));
    setText(ui->CIA1CRA, toHexStr(cia.timerCIA1.readCRA()));
    setText(ui->CIA1CRB, toHexStr(cia.timerCIA1.readCRB()));
    setText(ui->CIA2CRA, toHexStr(cia.timerCIA2.readCRA()));
    setText(ui->CIA2CRB, toHexStr(cia.timerCIA2.readCRB()));
    setText(ui->timer1ALatch, toHexStr(cia.timerCIA1.latchA));
    setText(ui->timer1BLatch, toHexStr(cia.timerCIA1.latchB));
    setText(ui->timer2ALatch, toHexStr(cia.timerCIA2.latchA));
    setText(ui->timer2BLatch, toHexStr(cia.timerCIA2.latchB));
    setText(ui->timer1ACounter, toHexStr(cia.timerCIA1.counterA));
    setText(ui->timer1BCounter, toHexStr(cia.timerCIA1.counterB));
    setText(ui->timer2ACounter, toHexStr(cia.timerCIA2.counterA));
    setText(ui->timer2BCounter, toHexStr(cia.timerCIA2.counterB));
    ui->CR1A5->setCurrentIndex(cia.timerCIA1.inmodeA);
    ui->CR1B5->setCurrentIndex(cia.timerCIA1.inmodeB);
    ui->CR2A5->setCurrentIndex(cia.timerCIA2.inmodeA);
    ui->CR2B5->setCurrentIndex(cia.timerCIA2.inmodeB);
    ui->CR1A3->setCurrentIndex(cia.timerCIA1.oneshotA ? 1 : 0);
    ui->CR1B3->setCurrentIndex(cia.timerCIA1.oneshotB ? 1 : 0);
    ui->CR2A3->setCurrentIndex(cia.timerCIA2.oneshotA ? 1 : 0);
    ui->CR2B3->setCurrentIndex(cia.timerCIA2.oneshotB ? 1 : 0);
    ui->CR1A0->setChecked(cia.timerCIA1.startedA);
    ui->CR1B0->setChecked(cia.timerCIA1.startedB);
    ui->CR2A0->setChecked(cia.timerCIA2.startedA);
    ui->CR2B0->setChecked(cia.timerCIA2.startedB);
    ui->IRQ->setChecked(cia.IRQ);
    ui->NMI->setChecked(cia.NMI);
    ui->IRQ0Data->setChecked(cia.ICRData1 & 0x01);
    ui->IRQ1Data->setChecked(cia.ICRData1 & 0x02);
    ui->NMI0Data->setChecked(cia.ICRData2 & 0x01);
    ui->NMI1Data->setChecked(cia.ICRData2 & 0x02);
    ui->IRQ0Mask->setChecked(cia.ICRMask1 & 0x01);
    ui->IRQ1Mask->setChecked(cia.ICRMask1 & 0x02);
    ui->NMI0Mask->setChecked(cia.ICRMask2 & 0x01);
    ui->NMI1Mask->setChecked(cia.ICRMask2 & 0x02);
}
