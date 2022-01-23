#include "video_widget.h"

#include <QPainter>
#include <QDebug>

#include <array>
#include <iostream>

extern const QList<QRgb> colorPalette;

VideoWidget::VideoWidget(QWidget *parent, int width, int height, std::vector<uint8_t> *videoBuffer)
    : QWidget(parent),
      videoBuffer(videoBuffer),
      width(width), height(height) {

    for(int i = 0; i < 16; i++) {
        // https://www.c64-wiki.com/wiki/Color
        colorPalette.push_back(qRgb(0x00, 0x00, 0x00));
        colorPalette.push_back(qRgb(0xFF, 0xFF, 0xFF));
        colorPalette.push_back(qRgb(0x88, 0x00, 0x00));
        colorPalette.push_back(qRgb(0xAA, 0xFF, 0xEE));
        colorPalette.push_back(qRgb(0xCC, 0x44, 0xCC));
        colorPalette.push_back(qRgb(0x00, 0xCC, 0x55));
        colorPalette.push_back(qRgb(0x00, 0x00, 0xAA));
        colorPalette.push_back(qRgb(0xEE, 0xEE, 0x77));
        colorPalette.push_back(qRgb(0xDD, 0x88, 0x55));
        colorPalette.push_back(qRgb(0x66, 0x44, 0x00));
        colorPalette.push_back(qRgb(0xFF, 0x77, 0x77));
        colorPalette.push_back(qRgb(0x33, 0x33, 0x33));
        colorPalette.push_back(qRgb(0x77, 0x77, 0x77));
        colorPalette.push_back(qRgb(0xAA, 0xFF, 0x66));
        colorPalette.push_back(qRgb(0x00, 0x88, 0xFF));
        colorPalette.push_back(qRgb(0xBB, 0xBB, 0xBB));
    }
}

VideoWidget::~VideoWidget() {

}

void VideoWidget::updateUI() {
    repaint();
}

void VideoWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QImage image(videoBuffer->data(), width, height, QImage::Format::Format_Indexed8);
    image.setColorTable(colorPalette);

    float scale = std::min(static_cast<float>(QWidget::width()) / width, static_cast<float>(QWidget::height()) / height);
    painter.scale(scale, scale);
    float x = (QWidget::width() - scale * width) / 2 / scale;
    float y = (QWidget::height() - scale * height) / 2 / scale;

    // dynamic minification:
    if (scale < 0.5f)
        painter.setRenderHint(QPainter::RenderHint::SmoothPixmapTransform, true);

    painter.drawImage(x, y, image);
}
