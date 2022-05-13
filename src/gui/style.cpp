#include "style.h"

#include "main_window.h"

#include <QApplication>
#include <QPalette>
#include <QWidget>
#include <QFont>
#include <QMouseEvent>

#include <iostream>

const auto backDark   = QColor( 19,  23,  26);
const auto backHalfDark=QColor( 26,  30,  38);
const auto backBright = QColor( 46,  51,  54);
const auto textBright = QColor(175, 183, 188);
const auto accent     = QColor(255, 102,   0);
const auto undef      = QColor(255,   0, 255);

void applyStyle(QApplication& app) {
    QPalette p;
    p.setColor(QPalette::Window, backBright);
    p.setColor(QPalette::WindowText, textBright);
    p.setColor(QPalette::ColorGroup::All, QPalette::Base, undef);
    p.setColor(QPalette::ColorGroup::Active, QPalette::Base, backDark);
    p.setColor(QPalette::ColorGroup::Inactive, QPalette::Base, backDark);
    p.setColor(QPalette::ColorGroup::Disabled, QPalette::Base, backHalfDark);
    p.setColor(QPalette::AlternateBase, undef);
    p.setColor(QPalette::ToolTipBase, backDark);
    p.setColor(QPalette::ToolTipText, textBright);
    p.setColor(QPalette::PlaceholderText, undef);
    p.setColor(QPalette::Text, textBright);
    p.setColor(QPalette::Button, backDark);
    p.setColor(QPalette::ButtonText, textBright);
    p.setColor(QPalette::BrightText, undef);
    p.setColor(QPalette::Highlight, backBright);
    p.setColor(QPalette::HighlightedText, textBright);
    QApplication::setStyle("Fusion");
    QApplication::setPalette(p);
    auto mono = QFont("Courier New");
    mono.setStyleHint(QFont::StyleHint::Monospace);
    QString qss;
    qss += "QMenuBar:item:selected { background: " + backDark.name() + "; }\n";
    qss += "QLineEdit,QPlainTextEdit,QListWidget { font-family: '" + mono.defaultFamily() + "'; color: " + accent.name() + "; }\n";
    qss += "QLineEdit:read-only { color: " + textBright.name() + "; }\n";
    qss += "QSpinBox { color: " + accent.name() + "; }\n";
    qss += "QGroupBox { border: 1px solid " + backDark.name() + "; border-radius: 2.5px; margin-top: 1em; }\n";
    qss += "QGroupBox:title { margin-top: 0.4em; subcontrol-origin: margin; subcontrol-position: top center; background-color: " + backBright.name() + "; }\n";
    qss += "QPushButton:flat { background-color: transparent; border: none;}\n";
    app.setStyleSheet(qss);
}



TitlebarWidget::TitlebarWidget(QMainWindow *parent) : parent(parent), QWidget(parent->centralWidget()) {
    init();
}
TitlebarWidget::TitlebarWidget(QWidget *parent) : parent(parent), QWidget(parent) {
    init();
}

void TitlebarWidget::init() {
    setMouseTracking(true);
    QObject::connect(parent, &QWidget::windowTitleChanged, [this](const QString& title) {
        titleText->setText(title);
    });
    auto mainWindow = dynamic_cast<MainWindow*>(parent);
    auto icon = new QLabel(parent);
    icon->setPixmap(QPixmap(":/icon-512.png"));
    icon->setScaledContents(true);
    icon->move(2, 2);
    icon->resize(TitlebarHeight - 4, TitlebarHeight - 4);
    icon->lower();

    if (mainWindow) {
        minimize = new QPushButton(parent);
        minimize->setIcon(QIcon(":/icons/minimize.png"));
        minimize->move(parent->width() - 3 * TitlebarHeight, 0);
        minimize->resize(TitlebarHeight, TitlebarHeight);
        minimize->setIconSize({TitlebarHeight / 2, TitlebarHeight / 2});
        minimize->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        buttons.push_back(minimize);
        QObject::connect(minimize, &QPushButton::clicked, [this]() {
            parent->setWindowState(parent->windowState() | Qt::WindowMinimized);
        });
        minimize->setFlat(true);
        maximize = new QPushButton(parent);
        maximize->setIcon(QIcon(":/icons/maximize.png"));
        maximize->move(parent->width() - 2 * TitlebarHeight, 0);
        maximize->resize(TitlebarHeight, TitlebarHeight);
        maximize->setIconSize({TitlebarHeight / 2, TitlebarHeight / 2});
        maximize->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        maximize->setFlat(true);
        buttons.push_back(maximize);
        QObject::connect(maximize, &QPushButton::clicked, [this]() {
            if (parent->isMaximized()) parent->showNormal();
            else  parent->showMaximized();
            maximize->setIcon(QIcon(parent->isMaximized() ? ":/icons/restore.png" : ":/icons/maximize.png"));
        });
    }
    close = new QPushButton(parent);
    close->setIcon(QIcon(":/icons/close.png"));
    close->move(parent->width() - TitlebarHeight, 0);
    close->resize(TitlebarHeight, TitlebarHeight);
    close->setIconSize({TitlebarHeight / 2, TitlebarHeight / 2});
    close->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    close->setFlat(true);
    buttons.push_back(close);
    QObject::connect(close, &QPushButton::clicked, [this]() {
        parent->close();
    });

    titleText = new QLabel(parent);
    titleText->setStyleSheet("QLabel { font-size: " + QString::fromStdString(std::to_string(TitlebarFont)) + "px; color: " + accent.name() + "; background-color: " + backDark.name() + "; }");
    titleText->setAlignment(Qt::AlignmentFlag::AlignCenter);
    titleText->move(0, 0);
    titleText->resize(parent->width(), TitlebarHeight);
    titleText->lower();
    titleText->setText(parent->windowTitle());

    if (mainWindow) {
        mainWindow->registerResizeCallback([this](QResizeEvent* event) {
            resize(event->size());
            titleText->resize(event->size().width(), TitlebarHeight);
            lower();
            if (!firstResize && !isFullScreen()) {
                for(auto b : buttons) {
                    b->move(b->pos() + QPoint{event->size().width() - event->oldSize().width(), 0});
                }
            } else firstResize = false;
        });
    }
}

void TitlebarWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        mouseDown = true;
        mousePos = (event->globalPosition() - parent->pos());
    }
    return QWidget::mousePressEvent(event);
}
void TitlebarWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        mouseDown = false;
    }
    return QWidget::mouseReleaseEvent(event);
}
void TitlebarWidget::mouseMoveEvent(QMouseEvent *event) {
    if (mouseDown) {
        if (dynamic_cast<MainWindow*>(parent)) {
            mousePos = {mousePos.x() / parent->width(), mousePos.y() / parent->height()};
            parent->setWindowState(parent->windowState() & ~Qt::WindowFullScreen);
            mousePos = {mousePos.x() * parent->width(), mousePos.y() * parent->height()};
            if (maximize) maximize->setIcon(QIcon(parent->windowState() & Qt::WindowFullScreen ? ":/icons/restore.png" : ":/icons/maximize.png"));
        }
        auto newPos = event->globalPosition() - mousePos;
        parent->move(newPos.toPoint()); // ignore clang warning, dynamic_cast in fact returns null, even if pointer is not null
    }
    return QWidget::mouseMoveEvent(event);
}
void TitlebarWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && dynamic_cast<MainWindow*>(parent)) {
        if (parent->isMaximized()) parent->showNormal();
        else  parent->showMaximized();
        maximize->setIcon(QIcon(parent->isMaximized() ? ":/icons/restore.png" : ":/icons/maximize.png"));
    }
}

void TitlebarWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);

    auto mainWindow = dynamic_cast<MainWindow*>(parent);
    if(!mainWindow) {
        if (minimize) minimize->move(event->size().width() - 3 * TitlebarHeight, 0);
        if (maximize) maximize->move(event->size().width() - 2 * TitlebarHeight, 0);
        if (close) close->move(event->size().width() - TitlebarHeight, 0);
        titleText->resize(event->size().width(), TitlebarHeight);
    }
}

TitlebarWidget* addDarkTitlebar(QWidget *window) {
    window->setWindowFlags(window->windowFlags() | Qt::Widget | Qt::FramelessWindowHint);
    auto size = window->size();
    window->resize(size.width(), size.height() + TitlebarHeight);

    auto mainWindow = dynamic_cast<QMainWindow*>(window);
    for(auto o : (mainWindow ? mainWindow->centralWidget() : window)->children()) {
        if (!o->isWidgetType()) continue;
        auto& c = *static_cast<QWidget*>(o);
        c.move(c.pos() + QPoint{0, TitlebarHeight});
    }

    TitlebarWidget *titlebar;
    if (mainWindow) titlebar = new TitlebarWidget(mainWindow);
    else            titlebar = new TitlebarWidget(window);
    titlebar->move(0, 0);
    titlebar->resize(size);
    titlebar->lower();

    return titlebar;
}

#include "style.moc"
