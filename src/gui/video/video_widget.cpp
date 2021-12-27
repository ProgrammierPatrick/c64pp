#include "video_widget.h"

#include <QPainter>
#include <QDebug>

#include <array>

extern const QList<QRgb> colorPalette;

VideoWidget::VideoWidget(QWidget *parent, int width, int height, std::vector<uint8_t> *videoBuffer)
    : QWidget(parent),
      width(width), height(height),
      videoBuffer(videoBuffer) {

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

    // painter.setRenderHint(QPainter::RenderHint::SmoothPixmapTransform, true);

    painter.drawImage(x, y, image);
}

const QList<QRgb> colorPalette {
    // https://www.c64-wiki.com/wiki/Color
    qRgb(0x00, 0x00, 0x00),
    qRgb(0xFF, 0xFF, 0xFF),
    qRgb(0x88, 0x00, 0x00),
    qRgb(0xAA, 0xFF, 0xEE),
    qRgb(0xCC, 0x44, 0xCC),
    qRgb(0x00, 0xCC, 0x55),
    qRgb(0x00, 0x00, 0xAA),
    qRgb(0xEE, 0xEE, 0x77),
    qRgb(0xDD, 0x88, 0x55),
    qRgb(0x66, 0x44, 0x00),
    qRgb(0xFF, 0x77, 0x77),
    qRgb(0x33, 0x33, 0x33),
    qRgb(0x77, 0x77, 0x77),
    qRgb(0xAA, 0xFF, 0x66),
    qRgb(0x00, 0x88, 0xFF),
    qRgb(0xBB, 0xBB, 0xBB),
};
