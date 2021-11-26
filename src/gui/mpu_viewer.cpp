#include "mpu_viewer.h"
#include "ui_mpu_viewer.h"

#include "text_utils.h"

MPUViewer::MPUViewer(QWidget *parent, C64Runner *c64Runner) :
    QMainWindow(parent),
    ui(new Ui::MPUViewer),
    c64Runner(c64Runner)
{
    ui->setupUi(this);

    // disable resizing
    setFixedSize(size());

    updateC64();
}

MPUViewer::~MPUViewer()
{
    delete ui;
}

void MPUViewer::updateC64() {
    auto& mpu = c64Runner->c64->mpu;
    ui->lineEditA->setText(QString::fromStdString(toHexStr(mpu.A)));
    ui->lineEditX->setText(QString::fromStdString(toHexStr(mpu.X)));
    ui->lineEditY->setText(QString::fromStdString(toHexStr(mpu.Y)));
    ui->lineEditS->setText(QString::fromStdString(toHexStr(mpu.S)));
    ui->lineEditPC->setText(QString::fromStdString(toHexStr(static_cast<uint16_t>(static_cast<int>(mpu.PCH) << 8 | mpu.PCL))));
}
