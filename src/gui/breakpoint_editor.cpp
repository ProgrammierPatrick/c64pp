#include "breakpoint_editor.h"
#include "ui_breakpoint_editor.h"
#include "text_utils.h"
#include "main_window.h"

#include <algorithm>

BreakpointEditor::BreakpointEditor(MainWindow *parent, C64Runner *c64Runner) :
    QMainWindow(parent),
    ui(new Ui::BreakpointEditor),
    c64Runner(c64Runner),
    mainWindow(parent)
{
    ui->setupUi(this);
    updateC64();
    // disable resizing
    setFixedSize(size());
    ui->inputLine->setFocus();
    ui->inputLine->setMaxLength(4);

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
                this->c64Runner->c64->breakPoints.instructionBreakpoints.push_back(addr);
                std::sort(vec.begin(), vec.end());
            }
        }
        ui->inputLine->clear();
        ui->inputLine->setFocus();
        updateC64();
    });

}

BreakpointEditor::~BreakpointEditor()
{
    delete ui;
}

void BreakpointEditor::updateC64() {
    ui->list->sortItems(Qt::AscendingOrder);
    auto& vec = this->c64Runner->c64->breakPoints.instructionBreakpoints;

    bool refresh = false;
    for (int i = 0; i < vec.size(); i++) {
        if (toHexStr(vec[i]) != ui->list->item(i)->text().toStdString()) {
                ui->list->clear();
                refresh = true;
        }
    }

    if (refresh) {
        for (auto val : vec) {
            ui->list->addItem(QString::fromStdString(toHexStr(val)));
        }
    }
}
