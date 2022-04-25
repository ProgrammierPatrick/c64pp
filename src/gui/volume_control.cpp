#include "volume_control.h"
#include "ui_volume_control.h"
#include "main_window.h"

#include "../emu/text_utils.h"

#include <algorithm>

VolumeControl::VolumeControl(MainWindow *parent, C64Runner *c64Runner) :
    QMainWindow(parent),
    ui(new Ui::VolumeControl),
    c64Runner(c64Runner),
    mainWindow(parent)
{
    ui->setupUi(this);
    // disable resizing
    setFixedSize(size());
    ui->slider->setRange(0, 100);
    ui->slider->setSliderPosition(mainWindow->volumeIntensity);
    mainWindow->setVolume(static_cast<double>(ui->slider->sliderPosition())/100);
    if (ui->slider->sliderPosition() == 0) ui->mute->setIcon(QIcon(":/icons/mute.png"));
    else if (ui->slider->sliderPosition() < 33) ui->mute->setIcon(QIcon(":/icons/low-vol.png"));
    else if (ui->slider->sliderPosition() < 67) ui->mute->setIcon(QIcon(":/icons/med-vol.png"));
    else ui->mute->setIcon(QIcon(":/icons/high-vol.png"));
    ui->mute->setIconSize(QSize(30,30));
    ui->volume->setText(QString::number(ui->slider->sliderPosition()));

    // mute button is clicked
    QObject::connect(ui->mute, &QPushButton::clicked, [this] {
        // mute
        if (ui->mute->isChecked()) {
            mainWindow->setVolume(0);
            ui->mute->setIcon(QIcon(":/icons/mute.png"));
        }
        // unmute
        if (!ui->mute->isChecked()) {
            mainWindow->setVolume(static_cast<double>(ui->slider->sliderPosition())/100);
            if (ui->slider->sliderPosition() == 0) ui->mute->setIcon(QIcon(":/icons/mute.png"));
            else if (ui->slider->sliderPosition() < 33) ui->mute->setIcon(QIcon(":/icons/low-vol.png"));
            else if (ui->slider->sliderPosition() < 67) ui->mute->setIcon(QIcon(":/icons/med-vol.png"));
            else ui->mute->setIcon(QIcon(":/icons/high-vol.png"));
        }
    });

    // slider is moved
    QObject::connect(ui->slider, &QSlider::valueChanged, [this] {
        // set icons depending on slider position
        if (ui->slider->sliderPosition() == 0) ui->mute->setIcon(QIcon(":/icons/mute.png"));
        else if (ui->slider->sliderPosition() < 33) ui->mute->setIcon(QIcon(":/icons/low-vol.png"));
        else if (ui->slider->sliderPosition() < 67) ui->mute->setIcon(QIcon(":/icons/med-vol.png"));
        else ui->mute->setIcon(QIcon(":/icons/high-vol.png"));

        ui->volume->setText(QString::number(ui->slider->sliderPosition()));
        mainWindow->setVolume(static_cast<double>(ui->slider->sliderPosition())/100);
    });



}

VolumeControl::~VolumeControl()
{
    delete ui;
}
