#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>

class QApplication;
class TitlebarWidget;

void applyStyle(QApplication& app);
TitlebarWidget* addDarkTitlebar(QWidget *window);

const int TitlebarHeight = 25; // px
const int TitlebarFont   = 15; // px

class TitlebarWidget : public QWidget {
    Q_OBJECT

public:
    TitlebarWidget(QMainWindow *parent);
    TitlebarWidget(QWidget *parent);
    void init();
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private:
    bool mouseDown = false;
    QPointF mousePos = {};
    QWidget *parent;
    QLabel *titleText;
    std::vector<QPushButton*> buttons = {};
    QPushButton *maximize = nullptr;
    QPushButton *minimize = nullptr;
    QPushButton *close = nullptr;
    bool firstResize = true;


};
