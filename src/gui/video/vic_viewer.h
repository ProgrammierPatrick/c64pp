#pragma once

#include "../c64_runner.h"

#include <QMainWindow>
#include <vector>

namespace Ui {
class VICViewer;
}

class MainWindow;
class VideoWidget;

class VICViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit VICViewer(MainWindow *parent, C64Runner *c64Runner);
    ~VICViewer();

    void updateC64();

private:
    Ui::VICViewer *ui;
    C64Runner *c64Runner;
    MainWindow *mainWindow;
    VideoWidget *charsetWidget;
    std::vector<uint8_t> charsetScreen;
    VideoWidget *matrixWidget;
    std::vector<uint8_t> matrixScreen;
    std::array<std::pair<VideoWidget*, std::vector<uint8_t>>,8> spriteScreens;
};

