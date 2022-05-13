#include "vic_viewer.h"
#include "ui_vic_viewer.h"

#include "video_widget.h"

#include "../style.h"
#include "../main_window.h"

#include "../../emu/text_utils.h"

#include <QLayout>

VICViewer::VICViewer(MainWindow *parent, C64Runner *c64Runner) :
    QMainWindow(parent),
    ui(new Ui::VICViewer),
    c64Runner(c64Runner),
    mainWindow(parent)
{
    ui->setupUi(this);
    addDarkTitlebar(this);
    setFixedSize(size());

    charsetScreen.resize(8 * 8 * 16 * 16);
    charsetWidget = new VideoWidget(ui->charsetFrame, 8 * 16, 8 * 16, &charsetScreen);
    ui->charsetFrame->layout()->addWidget(charsetWidget);

    matrixScreen.resize(8 * 8 * 40 * 25);
    matrixWidget = new VideoWidget(ui->charsetFrame, 8 * 40, 25 * 8, &matrixScreen);
    ui->matrixFrame->layout()->addWidget(matrixWidget);

    auto addSpriteScreen = [](auto& screen, auto box) {
        auto& [widget, data] = screen;
        data.resize(24 * 21);
        widget = new VideoWidget(box, 24, 21, &data);
        box->layout()->addWidget(widget);
    };
    addSpriteScreen(spriteScreens[0], ui->spriteFrame_0);
    addSpriteScreen(spriteScreens[1], ui->spriteFrame_1);
    addSpriteScreen(spriteScreens[2], ui->spriteFrame_2);
    addSpriteScreen(spriteScreens[3], ui->spriteFrame_3);
    addSpriteScreen(spriteScreens[4], ui->spriteFrame_4);
    addSpriteScreen(spriteScreens[5], ui->spriteFrame_5);
    addSpriteScreen(spriteScreens[6], ui->spriteFrame_6);
    addSpriteScreen(spriteScreens[7], ui->spriteFrame_7);

    auto setupSpinBox = [this](QSpinBox* box, std::function<void(int)> func, int min = 0, int max = 15) {
        box->setMinimum(min);
        box->setMaximum(max);
        QObject::connect(box, &QSpinBox::valueChanged, [func,this](int v) {
            func(v);
            mainWindow->updateUI();
        });
    };
    auto setupColorSpinBox = [this](QSpinBox* box, std::function<void(int)> func, int min = 0, int max = 15) {
        box->setMinimum(min);
        box->setMaximum(max);
        QObject::connect(box, &QSpinBox::valueChanged, [func,box,this](int v) {
            func(v);
            box->setStyleSheet("QSpinBox { background-color: " + QColor(matrixWidget->getColor(v)).name() + "; }");
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

    setupColorSpinBox(ui->borderColor, [this](int v) { this->c64Runner->c64->vic.borderColor = v; });
    setupColorSpinBox(ui->bgColor0,    [this](int v) { this->c64Runner->c64->vic.backgroundColors[0] = v; });
    setupColorSpinBox(ui->bgColor1,    [this](int v) { this->c64Runner->c64->vic.backgroundColors[1] = v; });
    setupColorSpinBox(ui->bgColor2,    [this](int v) { this->c64Runner->c64->vic.backgroundColors[2] = v; });
    setupColorSpinBox(ui->bgColor3,    [this](int v) { this->c64Runner->c64->vic.backgroundColors[3] = v; });
    setupColorSpinBox(ui->spriteMultiCol_0, [this](int v) { this->c64Runner->c64->vic.sprites.spriteMulticolor0 = v; });
    setupColorSpinBox(ui->spriteMultiCol_1, [this](int v) { this->c64Runner->c64->vic.sprites.spriteMulticolor1 = v; });

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
        this->c64Runner->c64->cia.vicBank = (v >> 14) & 0x03;
    });
    setupLineEdit16(ui->cbAddr, [this](uint16_t v) {
        this->c64Runner->c64->vic.charGenMemoryPosition = (v >> 11) & 0x7;
        this->c64Runner->c64->cia.vicBank = (v >> 14) & 0x03;
    });

    QObject::connect(ui->bank15, &QCheckBox::stateChanged, [this]() {
        if (ui->bank15->isChecked())
            this->c64Runner->c64->cia.vicBank |= 0x02;
        else
            this->c64Runner->c64->cia.vicBank &= ~0x02;
        mainWindow->updateUI();
    });
    QObject::connect(ui->bank14, &QCheckBox::stateChanged, [this]() {
        if (ui->bank14->isChecked())
            this->c64Runner->c64->cia.vicBank |= 0x01;
        else
            this->c64Runner->c64->cia.vicBank &= ~0x01;
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

    auto setupSprite = [&](int i, QCheckBox *enabled, QSpinBox *x, QSpinBox *y, QSpinBox *color, QSpinBox *mc, QSpinBox *mcbase, QCheckBox *xExp, QCheckBox *yExp, QCheckBox *spriteCol, QCheckBox *dataCol, QCheckBox *prio, QCheckBox *multiCol) {
        QObject::connect(enabled, &QCheckBox::stateChanged, [this,i,enabled]() { this->c64Runner->c64->vic.sprites.spriteData[i].spriteEnabled = enabled->isChecked(); mainWindow->updateUI(); });
        setupSpinBox(x,     [this,i](int val) { this->c64Runner->c64->vic.sprites.spriteData[i].xCoord = val; }, 0, 511);
        setupSpinBox(y,     [this,i](int val) { this->c64Runner->c64->vic.sprites.spriteData[i].yCoord = val; }, 0, 255);
        setupColorSpinBox(color, [this,i](int val) { this->c64Runner->c64->vic.sprites.spriteData[i].spriteColor = val; }, 0, 15);
        setupSpinBox(mc,    [this,i](int val) { this->c64Runner->c64->vic.sprites.spriteData[i].spriteDataCounter = val; }, 0, 65);
        setupSpinBox(mcbase,[this,i](int val) { this->c64Runner->c64->vic.sprites.spriteData[i].spriteDataCounterBase = val; }, 0, 65);
        QObject::connect(xExp,      &QCheckBox::stateChanged, [this,i,xExp]()      { this->c64Runner->c64->vic.sprites.spriteData[i].spriteXExpansion      = xExp->isChecked();      mainWindow->updateUI(); });
        QObject::connect(yExp,      &QCheckBox::stateChanged, [this,i,yExp]()      { this->c64Runner->c64->vic.sprites.spriteData[i].spriteYExpansion      = yExp->isChecked();      mainWindow->updateUI(); });
        QObject::connect(spriteCol, &QCheckBox::stateChanged, [this,i,spriteCol]() { this->c64Runner->c64->vic.sprites.spriteData[i].spriteSpriteCollision = spriteCol->isChecked(); mainWindow->updateUI(); });
        QObject::connect(dataCol,   &QCheckBox::stateChanged, [this,i,dataCol]()   { this->c64Runner->c64->vic.sprites.spriteData[i].spriteDataCollision   = dataCol->isChecked();   mainWindow->updateUI(); });
        QObject::connect(prio,      &QCheckBox::stateChanged, [this,i,prio]()      { this->c64Runner->c64->vic.sprites.spriteData[i].spriteDataPriority    = prio->isChecked();      mainWindow->updateUI(); });
        QObject::connect(multiCol,  &QCheckBox::stateChanged, [this,i,multiCol]()  { this->c64Runner->c64->vic.sprites.spriteData[i].spriteMulticolor      = multiCol->isChecked();  mainWindow->updateUI(); });
    };
    setupSprite(0, ui->spriteEnabled_0, ui->spriteX_0, ui->spriteY_0, ui->spriteColor_0, ui->spriteMC_0, ui->spriteMCBASE_0, ui->spriteXExpand_0, ui->spriteYExpand_0, ui->spriteSpriteCollision_0, ui->spriteDataCollision_0, ui->spritePriority_0, ui->spriteMulticolor_0);
    setupSprite(1, ui->spriteEnabled_1, ui->spriteX_1, ui->spriteY_1, ui->spriteColor_1, ui->spriteMC_1, ui->spriteMCBASE_1, ui->spriteXExpand_1, ui->spriteYExpand_1, ui->spriteSpriteCollision_1, ui->spriteDataCollision_1, ui->spritePriority_1, ui->spriteMulticolor_1);
    setupSprite(2, ui->spriteEnabled_2, ui->spriteX_2, ui->spriteY_2, ui->spriteColor_2, ui->spriteMC_2, ui->spriteMCBASE_2, ui->spriteXExpand_2, ui->spriteYExpand_2, ui->spriteSpriteCollision_2, ui->spriteDataCollision_2, ui->spritePriority_2, ui->spriteMulticolor_2);
    setupSprite(3, ui->spriteEnabled_3, ui->spriteX_3, ui->spriteY_3, ui->spriteColor_3, ui->spriteMC_3, ui->spriteMCBASE_3, ui->spriteXExpand_3, ui->spriteYExpand_3, ui->spriteSpriteCollision_3, ui->spriteDataCollision_3, ui->spritePriority_3, ui->spriteMulticolor_3);
    setupSprite(4, ui->spriteEnabled_4, ui->spriteX_4, ui->spriteY_4, ui->spriteColor_4, ui->spriteMC_4, ui->spriteMCBASE_4, ui->spriteXExpand_4, ui->spriteYExpand_4, ui->spriteSpriteCollision_4, ui->spriteDataCollision_4, ui->spritePriority_4, ui->spriteMulticolor_4);
    setupSprite(5, ui->spriteEnabled_5, ui->spriteX_5, ui->spriteY_5, ui->spriteColor_5, ui->spriteMC_5, ui->spriteMCBASE_5, ui->spriteXExpand_5, ui->spriteYExpand_5, ui->spriteSpriteCollision_5, ui->spriteDataCollision_5, ui->spritePriority_5, ui->spriteMulticolor_5);
    setupSprite(6, ui->spriteEnabled_6, ui->spriteX_6, ui->spriteY_6, ui->spriteColor_6, ui->spriteMC_6, ui->spriteMCBASE_6, ui->spriteXExpand_6, ui->spriteYExpand_6, ui->spriteSpriteCollision_6, ui->spriteDataCollision_6, ui->spritePriority_6, ui->spriteMulticolor_6);
    setupSprite(7, ui->spriteEnabled_7, ui->spriteX_7, ui->spriteY_7, ui->spriteColor_7, ui->spriteMC_7, ui->spriteMCBASE_7, ui->spriteXExpand_7, ui->spriteYExpand_7, ui->spriteSpriteCollision_7, ui->spriteDataCollision_7, ui->spritePriority_7, ui->spriteMulticolor_7);

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
    uint16_t vicY = (vic.VCBASE == vic.VC) ? (vic.VCBASE / 40 * 8 + vic.RC - 7) : (vic.VCBASE / 40 * 8 + vic.RC);
    uint16_t vicX = vic.VMLI * 8;
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 40; x++) {
            for (int i = 0; i < 8; i++) {
                auto c = vic.accessMem(((vic.videoMatrixMemoryPosition & 0xF) << 10) | (y * 40 + x));
                auto pixels = vic.backgroundGraphics.gAccess(c, y * 40 + x, i % 8);
                for (int j = 0; j < 8; j++) {
                    matrixScreen[y * 40 * 8 * 8 + i * 40 * 8 + x * 8 + j] = pixels.pixels[j];

                    // display red crosshair for current x/y coords
                    if (y * 8 + i == vicY || x * 8 + j == vicX)
                        matrixScreen[y * 40 * 8 * 8 + i * 40 * 8 + x * 8 + j] = 2;
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
    ui->spriteMultiCol_0->setValue(vic.sprites.spriteMulticolor0);
    ui->spriteMultiCol_1->setValue(vic.sprites.spriteMulticolor1);

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
    lineEditSet16(ui->vmAddr, ((vic.videoMatrixMemoryPosition & 0xF) << 10) | ((cia.vicBank & 0x3) << 14));
    lineEditSet16(ui->cbAddr, ((vic.charGenMemoryPosition & 0x7) << 11) | ((cia.vicBank & 0x3) << 14));

    ui->bank15->setChecked(cia.vicBank & 0x02);
    ui->bank14->setChecked(cia.vicBank & 0x01);
    ui->vm13->setChecked(vic.videoMatrixMemoryPosition & 0x08);
    ui->vm12->setChecked(vic.videoMatrixMemoryPosition & 0x04);
    ui->vm11->setChecked(vic.videoMatrixMemoryPosition & 0x02);
    ui->vm10->setChecked(vic.videoMatrixMemoryPosition & 0x01);
    ui->cb13->setChecked(vic.charGenMemoryPosition & 0x04);
    ui->cb12->setChecked(vic.charGenMemoryPosition & 0x02);
    ui->cb11->setChecked(vic.charGenMemoryPosition & 0x01);

    ui->DEN->setChecked(vic.displayEnable);

    auto updateSprite = [&](int i, QCheckBox *enabled, QSpinBox *x, QSpinBox *y, QSpinBox *color, QSpinBox *mc, QSpinBox *mcbase, QCheckBox *xExp, QCheckBox *yExp, QCheckBox *spriteCol, QCheckBox *dataCol, QCheckBox *prio, QCheckBox *multiCol) {
        auto& s = vic.sprites.spriteData[i];
        enabled->setChecked(s.spriteEnabled);
        x->setValue(s.xCoord);
        y->setValue(s.yCoord);
        color->setValue(s.spriteColor);
        mc->setValue(s.spriteDataCounter);
        mcbase->setValue(s.spriteDataCounterBase);
        xExp->setChecked(s.spriteXExpansion);
        yExp->setChecked(s.spriteYExpansion);
        spriteCol->setChecked(s.spriteSpriteCollision);
        dataCol->setChecked(s.spriteDataCollision);
        prio->setChecked(s.spriteDataPriority);
        multiCol->setChecked(s.spriteMulticolor);

        uint8_t p = vic.sprites.spritePAccess(i);
        auto& [widget, data] = spriteScreens[i];
        for (int y = 0; y < 21; y++) {
            for (int x = 0; x < 24; x++) {
                int pix_num = y * 24 + x;
                auto s_val = vic.accessMem(p << 6 | (pix_num / 8)).val;
                if (s.spriteMulticolor) {
                    // auto bits = s_val & (3 << (6 - ((pix_num / 2) % 4)));
                    uint8_t bits = (s_val >> (6 - 2 * ((pix_num / 2) % 4))) & 3;
                    data[pix_num] = (bits == 3) ? vic.sprites.spriteMulticolor1 : ((bits == 2) ? s.spriteColor : (bits == 1 ? vic.sprites.spriteMulticolor0 : 0));
                } else {
                    bool pixelSet = s_val & (1 << (7 - (pix_num % 8)));
                    data[pix_num] = pixelSet ? s.spriteColor : 0;
                }
            }
        }
        widget->updateUI();
    };
    updateSprite(0, ui->spriteEnabled_0, ui->spriteX_0, ui->spriteY_0, ui->spriteColor_0, ui->spriteMC_0, ui->spriteMCBASE_0, ui->spriteXExpand_0, ui->spriteYExpand_0, ui->spriteSpriteCollision_0, ui->spriteDataCollision_0, ui->spritePriority_0, ui->spriteMulticolor_0);
    updateSprite(1, ui->spriteEnabled_1, ui->spriteX_1, ui->spriteY_1, ui->spriteColor_1, ui->spriteMC_1, ui->spriteMCBASE_1, ui->spriteXExpand_1, ui->spriteYExpand_1, ui->spriteSpriteCollision_1, ui->spriteDataCollision_1, ui->spritePriority_1, ui->spriteMulticolor_1);
    updateSprite(2, ui->spriteEnabled_2, ui->spriteX_2, ui->spriteY_2, ui->spriteColor_2, ui->spriteMC_2, ui->spriteMCBASE_2, ui->spriteXExpand_2, ui->spriteYExpand_2, ui->spriteSpriteCollision_2, ui->spriteDataCollision_2, ui->spritePriority_2, ui->spriteMulticolor_2);
    updateSprite(3, ui->spriteEnabled_3, ui->spriteX_3, ui->spriteY_3, ui->spriteColor_3, ui->spriteMC_3, ui->spriteMCBASE_3, ui->spriteXExpand_3, ui->spriteYExpand_3, ui->spriteSpriteCollision_3, ui->spriteDataCollision_3, ui->spritePriority_3, ui->spriteMulticolor_3);
    updateSprite(4, ui->spriteEnabled_4, ui->spriteX_4, ui->spriteY_4, ui->spriteColor_4, ui->spriteMC_4, ui->spriteMCBASE_4, ui->spriteXExpand_4, ui->spriteYExpand_4, ui->spriteSpriteCollision_4, ui->spriteDataCollision_4, ui->spritePriority_4, ui->spriteMulticolor_4);
    updateSprite(5, ui->spriteEnabled_5, ui->spriteX_5, ui->spriteY_5, ui->spriteColor_5, ui->spriteMC_5, ui->spriteMCBASE_5, ui->spriteXExpand_5, ui->spriteYExpand_5, ui->spriteSpriteCollision_5, ui->spriteDataCollision_5, ui->spritePriority_5, ui->spriteMulticolor_5);
    updateSprite(6, ui->spriteEnabled_6, ui->spriteX_6, ui->spriteY_6, ui->spriteColor_6, ui->spriteMC_6, ui->spriteMCBASE_6, ui->spriteXExpand_6, ui->spriteYExpand_6, ui->spriteSpriteCollision_6, ui->spriteDataCollision_6, ui->spritePriority_6, ui->spriteMulticolor_6);
    updateSprite(7, ui->spriteEnabled_7, ui->spriteX_7, ui->spriteY_7, ui->spriteColor_7, ui->spriteMC_7, ui->spriteMCBASE_7, ui->spriteXExpand_7, ui->spriteYExpand_7, ui->spriteSpriteCollision_7, ui->spriteDataCollision_7, ui->spritePriority_7, ui->spriteMulticolor_7);

}
