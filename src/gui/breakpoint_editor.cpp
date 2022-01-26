#include "breakpoint_editor.h"
#include "ui_breakpoint_editor.h"

BreakpointEditor::BreakpointEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BreakpointEditor)
{
    ui->setupUi(this);
    updateC64();

}

BreakpointEditor::~BreakpointEditor()
{
    delete ui;
}

void BreakpointEditor::updateC64() {
    if(ui->enabled->isChecked() != ui->list->isEnabled()) {
        ui->list->setEnabled(ui->enabled->isChecked());
    }
}
