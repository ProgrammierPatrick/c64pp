#pragma once

#include <QWidget>
#include <QRgb>

#include <vector>
#include <cstdint>

class VideoWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent, int width, int height, std::vector<uint8_t> *videoBuffer);
    ~VideoWidget();

    void setVideoBuffer(std::vector<uint8_t> *videoBuffer) {
        this->videoBuffer = videoBuffer;
    }

    void paintEvent(QPaintEvent *event) override;

    void updateUI();

private:
    std::vector<uint8_t> *videoBuffer;
    int width, height;
};
