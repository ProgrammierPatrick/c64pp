#include "breakpoint_editor.h"
#include "ui_breakpoint_editor.h"

#include "main_window.h"

BreakpointEditor::BreakpointEditor(MainWindow *parent, C64Runner *c64Runner) :
    QMainWindow(parent),
    ui(new Ui::BreakpointEditor),
    c64Runner(c64Runner),
    mainWindow(parent)
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
