#include "vic_viewer.h"
#include "ui_vic_viewer.h"

#include "video_widget.h"
#include "../main_window.h"
#include "../text_utils.h"

#include <QLayout>

VICViewer::VICViewer(MainWindow *parent, C64Runner *c64Runner) :
    QMainWindow(parent),
    ui(new Ui::VICViewer),
    c64Runner(c64Runner),
    mainWindow(parent)
{
    ui->setupUi(this);

    charsetScreen.resize(8 * 8 * 16 * 16);
    charsetWidget = new VideoWidget(ui->charsetFrame, 8 * 16, 8 * 16, &charsetScreen);
    ui->charsetFrame->layout()->addWidget(charsetWidget);

    matrixScreen.resize(8 * 8 * 40 * 25);
    matrixWidget = new VideoWidget(ui->charsetFrame, 8 * 40, 25 * 8, &matrixScreen);
    ui->matrixFrame->layout()->addWidget(matrixWidget);

    auto setupSpinBox = [this](QSpinBox* box, std::function<void(int)> func, int min = 0, int max = 15) {
        box->setMinimum(min);
        box->setMaximum(max);
        QObject::connect(box, &QSpinBox::valueChanged, [func,this](int v) {
            func(v);
            mainWindow->updateUI();
        });
    };
    auto setupLineEdit16 = [this](QLineEdit* edit, std::function<void(uint16_t)> func) {
        edit->setMaxLength(4);
        edit->setInputMask("HHHH");
        QObject::connect(edit, &QLineEdit::editingFinished, [edit,func,this]() {
            auto text = edit->text().toStdString();
            if (isValidHex16(text)) {
                func(fromHexStr16(text));
                edit->setModified(false);
                mainWindow->updateUI();
            }
        });
    };

    setupSpinBox(ui->xCycle, [this](int v) {
        this->c64Runner->c64->vic.cycleInLine = v;
        this->c64Runner->c64->vic.x = VIC::firstCycleX + (v-1) * 8;
    }, 1, 63);
    setupSpinBox(ui->y,        [this](int v) { this->c64Runner->c64->vic.y = v; },                 0, VIC::lastY);
    setupSpinBox(ui->yCompare, [this](int v) { this->c64Runner->c64->vic.rasterCompareLine = v; }, 0, VIC::lastY);

    setupSpinBox(ui->borderColor, [this](int v) { this->c64Runner->c64->vic.borderColor = v; });
    setupSpinBox(ui->bgColor0,    [this](int v) { this->c64Runner->c64->vic.backgroundColors[0] = v; });
    setupSpinBox(ui->bgColor1,    [this](int v) { this->c64Runner->c64->vic.backgroundColors[1] = v; });
    setupSpinBox(ui->bgColor2,    [this](int v) { this->c64Runner->c64->vic.backgroundColors[2] = v; });
    setupSpinBox(ui->bgColor3,    [this](int v) { this->c64Runner->c64->vic.backgroundColors[3] = v; });

    setupSpinBox(ui->RC,     [this](int v) { this->c64Runner->c64->vic.RC = v; },     0, (1 << 3) - 1 );
    setupSpinBox(ui->VC,     [this](int v) { this->c64Runner->c64->vic.VC = v; },     0, (1 << 10) - 1);
    setupSpinBox(ui->VCBASE, [this](int v) { this->c64Runner->c64->vic.VCBASE = v; }, 0, (1 << 10) - 1);
    setupSpinBox(ui->VMLI,   [this](int v) { this->c64Runner->c64->vic.VMLI = v; },   0, (1 << 6) - 1);

    QObject::connect(ui->CSEl, &QCheckBox::stateChanged, [this]() { this->c64Runner->c64->vic.cSel = ui->CSEl->isChecked(); mainWindow->updateUI(); });
    QObject::connect(ui->RSEL, &QCheckBox::stateChanged, [this]() { this->c64Runner->c64->vic.rSel = ui->RSEL->isChecked(); mainWindow->updateUI(); });

    setupSpinBox(ui->xScroll,   [this](int v) { this->c64Runner->c64->vic.xScroll = v; },   0, (1 << 3) - 1);
    setupSpinBox(ui->yScroll,   [this](int v) { this->c64Runner->c64->vic.yScroll = v; },   0, (1 << 3) - 1);

    QObject::connect(ui->ECM, &QCheckBox::stateChanged, [this]() { this->c64Runner->c64->vic.extendedColorMode = ui->ECM->isChecked(); mainWindow->updateUI(); });
    QObject::connect(ui->BMM, &QCheckBox::stateChanged, [this]() { this->c64Runner->c64->vic.bitmapMode = ui->BMM->isChecked(); mainWindow->updateUI(); });
    QObject::connect(ui->MCM, &QCheckBox::stateChanged, [this]() { this->c64Runner->c64->vic.multiColorMode = ui->MCM->isChecked(); mainWindow->updateUI(); });

    setupLineEdit16(ui->vmAddr, [this](uint16_t v) {
        this->c64Runner->c64->vic.videoMatrixMemoryPosition = (v >> 10) & 0xF;
        this->c64Runner->c64->cia.PRA2 &= ~0x3;
        this->c64Runner->c64->cia.PRA2 |= (~v >> 14) & 0x3;
    });
    setupLineEdit16(ui->cbAddr, [this](uint16_t v) {
        this->c64Runner->c64->vic.charGenMemoryPosition = (v >> 11) & 0x7;
        this->c64Runner->c64->cia.PRA2 &= ~0x3;
        this->c64Runner->c64->cia.PRA2 |= (~v >> 14) & 0x3;
    });

    QObject::connect(ui->bank15, &QCheckBox::stateChanged, [this]() {
        if (ui->bank15->isChecked())
            this->c64Runner->c64->cia.PRA2 &= ~0x02;
        else
            this->c64Runner->c64->cia.PRA2 |= 0x02;
        mainWindow->updateUI();
    });
    QObject::connect(ui->bank14, &QCheckBox::stateChanged, [this]() {
        if (ui->bank14->isChecked())
            this->c64Runner->c64->cia.PRA2 &= ~0x01;
        else
            this->c64Runner->c64->cia.PRA2 |= 0x01;
        mainWindow->updateUI();
    });
    QObject::connect(ui->vm13, &QCheckBox::stateChanged, [this]() {
        if (ui->vm13->isChecked())
            this->c64Runner->c64->vic.videoMatrixMemoryPosition |= 0x8;
        else
            this->c64Runner->c64->vic.videoMatrixMemoryPosition &= ~0x8;
        mainWindow->updateUI();
    });
    QObject::connect(ui->vm12, &QCheckBox::stateChanged, [this]() {
        if (ui->vm12->isChecked())
            this->c64Runner->c64->vic.videoMatrixMemoryPosition |= 0x4;
        else
            this->c64Runner->c64->vic.videoMatrixMemoryPosition &= ~0x4;
        mainWindow->updateUI();
    });
    QObject::connect(ui->vm11, &QCheckBox::stateChanged, [this]() {
        if (ui->vm11->isChecked())
            this->c64Runner->c64->vic.videoMatrixMemoryPosition |= 0x2;
        else
            this->c64Runner->c64->vic.videoMatrixMemoryPosition &= ~0x2;
        mainWindow->updateUI();
    });
    QObject::connect(ui->vm10, &QCheckBox::stateChanged, [this]() {
        if (ui->vm10->isChecked())
            this->c64Runner->c64->vic.videoMatrixMemoryPosition |= 0x1;
        else
            this->c64Runner->c64->vic.videoMatrixMemoryPosition &= ~0x1;
        mainWindow->updateUI();
    });
    QObject::connect(ui->cb13, &QCheckBox::stateChanged, [this]() {
        if (ui->cb13->isChecked())
            this->c64Runner->c64->vic.charGenMemoryPosition |= 0x4;
        else
            this->c64Runner->c64->vic.charGenMemoryPosition &= ~0x4;
        mainWindow->updateUI();
    });
    QObject::connect(ui->cb12, &QCheckBox::stateChanged, [this]() {
        if (ui->cb12->isChecked())
            this->c64Runner->c64->vic.charGenMemoryPosition |= 0x2;
        else
            this->c64Runner->c64->vic.charGenMemoryPosition &= ~0x2;
        mainWindow->updateUI();
    });
    QObject::connect(ui->cb11, &QCheckBox::stateChanged, [this]() {
        if (ui->cb11->isChecked())
            this->c64Runner->c64->vic.charGenMemoryPosition |= 0x1;
        else
            this->c64Runner->c64->vic.charGenMemoryPosition &= ~0x1;
        mainWindow->updateUI();
    });

    QObject::connect(ui->DEN, &QCheckBox::stateChanged, [this]() {
        this->c64Runner->c64->vic.displayEnable = ui->DEN->isChecked();
        mainWindow->updateUI();
    });

    updateC64();
}

VICViewer::~VICViewer()
{
    delete ui;
}

void VICViewer::updateC64() {
    auto& vic = this->c64Runner->c64->vic;
    auto& cia = this->c64Runner->c64->cia;

    // render charset
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            for (int i = 0; i < 8; i++) {
                auto c = vic.bitmapMode ? ColoredVal(0x10, 0x2) : ColoredVal(y * 16 + x, 0xE);
                auto pixels = vic.backgroundGraphics.gAccess(c, y * 16 + x, i % 8);
                for (int j = 0; j < 8; j++) charsetScreen[y * 16 * 8 * 8 + i * 16 * 8 + x * 8 + j] = pixels.pixels[j];
            }
        }
    }

    // render matrixScreen
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 40; x++) {
            for (int i = 0; i < 8; i++) {
                uint16_t vicY = (vic.VCBASE == vic.VC) ? (vic.VCBASE / 40 * 8 + vic.RC - 7) : (vic.VCBASE / 40 * 8 + vic.RC);
                if ((y * 8 + i) == vicY) {
                    // red line for current y
                    for (int j = 0; j < 8; j++) matrixScreen[y * 40 * 8 * 8 + i * 40 * 8 + x * 8 + j] = 4;
                } else {
                    auto c = vic.accessMem(((vic.videoMatrixMemoryPosition & 0xF) << 10) | (y * 40 + x));
                    auto pixels = vic.backgroundGraphics.gAccess(c, y * 40 + x, i % 8);
                    for (int j = 0; j < 8; j++) matrixScreen[y * 40 * 8 * 8 + i * 40 * 8 + x * 8 + j] = pixels.pixels[j];
                }
            }
        }
    }

    charsetWidget->updateUI();
    matrixWidget->updateUI();

    ui->xCycle->setValue(vic.cycleInLine);
    ui->y->setValue(vic.y);
    ui->yCompare->setValue(vic.rasterCompareLine);

    ui->borderColor->setValue(vic.borderColor);
    ui->bgColor0->setValue(vic.backgroundColors[0]);
    ui->bgColor1->setValue(vic.backgroundColors[1]);
    ui->bgColor2->setValue(vic.backgroundColors[2]);
    ui->bgColor3->setValue(vic.backgroundColors[3]);

    ui->RC->setValue(vic.RC);
    ui->VC->setValue(vic.VC);
    ui->VCBASE->setValue(vic.VCBASE);
    ui->VMLI->setValue(vic.VMLI);

    ui->CSEl->setChecked(vic.cSel);
    ui->RSEL->setChecked(vic.rSel);

    ui->xScroll->setValue(vic.xScroll);
    ui->yScroll->setValue(vic.yScroll);

    ui->ECM->setChecked(vic.extendedColorMode);
    ui->BMM->setChecked(vic.bitmapMode);
    ui->MCM->setChecked(vic.multiColorMode);

    auto lineEditSet16 = [](QLineEdit* edit, uint16_t val) {
        auto text = toHexStr(val);
        if (edit->text().toStdString() != text && !edit->isModified()) {
            auto cursorPos = edit->cursorPosition();
            edit->setText(QString::fromStdString(text));
            edit->setCursorPosition(cursorPos);
        }
    };
    lineEditSet16(ui->vmAddr, ((vic.videoMatrixMemoryPosition & 0xF) << 10) | ((~cia.PRA2 & 0x3) << 14));
    lineEditSet16(ui->cbAddr, ((vic.charGenMemoryPosition & 0x7) << 11) | ((~cia.PRA2 & 0x3) << 14));

    ui->bank15->setChecked(~cia.PRA2 & 0x02);
    ui->bank14->setChecked(~cia.PRA2 & 0x01);
    ui->vm13->setChecked(vic.videoMatrixMemoryPosition & 0x08);
    ui->vm12->setChecked(vic.videoMatrixMemoryPosition & 0x04);
    ui->vm11->setChecked(vic.videoMatrixMemoryPosition & 0x02);
    ui->vm10->setChecked(vic.videoMatrixMemoryPosition & 0x01);
    ui->cb13->setChecked(vic.charGenMemoryPosition & 0x04);
    ui->cb12->setChecked(vic.charGenMemoryPosition & 0x02);
    ui->cb11->setChecked(vic.charGenMemoryPosition & 0x01);

    ui->DEN->setChecked(vic.displayEnable);
}
