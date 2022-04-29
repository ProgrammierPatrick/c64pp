#include "breakpoint_editor.h"
#include "ui_breakpoint_editor.h"
#include "main_window.h"

#include "../emu/text_utils.h"

#include <algorithm>

BreakpointEditor::BreakpointEditor(MainWindow *parent, C64Runner *c64Runner) :
    QMainWindow(parent),
    ui(new Ui::BreakpointEditor),
    c64Runner(c64Runner),
    mainWindow(parent)
{
    ui->setupUi(this);
    // disable resizing
    setFixedSize(size());
    ui->inputLine->setFocus();
    ui->inputLine->setMaxLength(4);
    bool enabledOld = false;

    QObject::connect(ui->inputLine, &QLineEdit::returnPressed, ui->add, &QPushButton::click);

    QObject::connect(ui->removeAll, &QPushButton::clicked, [this](bool b) {
        ui->list->clear();
        this->c64Runner->c64->breakPoints.instructionBreakpoints.clear();
        ui->inputLine->clear();
        ui->inputLine->setFocus();
        updateC64();
    });

    QObject::connect(ui->remove, &QPushButton::clicked, [this](bool b) {
        auto item = ui->list->currentItem();
        if (!item) return;
        ui->list->takeItem(ui->list->row(item));
        auto breakpoint = fromHexStr16(item->text().toStdString());
        auto& vec = this->c64Runner->c64->breakPoints.instructionBreakpoints;
        vec.erase(std::remove(vec.begin(), vec.end(), breakpoint), vec.end());
        ui->inputLine->clear();
        ui->inputLine->setFocus();
        updateC64();
    });

    QObject::connect(ui->add, &QPushButton::clicked, [this](bool b) {
        auto hex = padZeros(ui->inputLine->text().toStdString(), 4);
        auto& vec = this->c64Runner->c64->breakPoints.instructionBreakpoints;
        if (isValidHex16(hex)) {
            uint16_t addr = fromHexStr16(hex);
            if (std::find(vec.begin(), vec.end(), addr) == vec.end()) {
                ui->list->addItem(QString::fromStdString(toHexStr(addr)));
                vec.push_back(addr);
                std::sort(vec.begin(), vec.end());
            }
        }
        ui->inputLine->clear();
        ui->inputLine->setFocus();
        updateC64();
    });

    QObject::connect(ui->enabled, &QCheckBox::stateChanged, [this](bool b) {
        this->c64Runner->c64->breakPoints.enable = ui->enabled->isChecked();
        ui->inputLine->clear();
        ui->inputLine->setFocus();
        updateC64();
    });

    updateC64();
}

BreakpointEditor::~BreakpointEditor()
{
    delete ui;
}

void BreakpointEditor::updateC64() {
    ui->list->sortItems(Qt::AscendingOrder);
    auto& vec = this->c64Runner->c64->breakPoints.instructionBreakpoints;

    if (ui->enabled->isChecked() != this->c64Runner->c64->breakPoints.enable) {
        ui->enabled->setChecked(this->c64Runner->c64->breakPoints.enable);
    }

    bool refresh = false;
    if (vec.size() != ui->list->count()) {
        refresh = true;
    } else {
        for (int i = 0; i < vec.size(); i++) {
            if (toHexStr(vec[i]) != ui->list->item(i)->text().toStdString()) {
                refresh = true;
            }
        }
    }

    if (refresh) {
        ui->list->clear();
        for (auto val : vec) {
            ui->list->addItem(QString::fromStdString(toHexStr(val)));
        }
    }
}
