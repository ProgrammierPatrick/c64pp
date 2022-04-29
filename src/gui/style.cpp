#include "style.h"

#include <QApplication>
#include <QPalette>
#include <QWidget>
#include <QFont>

#include <iostream>

void applyStyle(QApplication& app) {
    QPalette p;
    auto backDark   = QColor( 19,  23,  26);
    auto backHalfDark=QColor( 26,  30,  38);
    auto backBright = QColor( 46,  51,  54);
    auto textBright = QColor(175, 183, 188);
    auto accent     = QColor(255, 102,   0);
    auto undef      = QColor(255,   0, 255);
    p.setColor(QPalette::Window, backBright);
    p.setColor(QPalette::WindowText, textBright);
    p.setColor(QPalette::ColorGroup::All, QPalette::Base, undef);
    p.setColor(QPalette::ColorGroup::Active, QPalette::Base, backDark);
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
    qss += "QSpinBox { color: " + accent.name() + "; }\n";
    qss += "QGroupBox { border: 1px solid " + backDark.name() + "; border-radius: 2.5px; margin-top: 1em; }\n";
    qss += "QGroupBox:title { margin-top: 0.4em; subcontrol-origin: margin; subcontrol-position: top center; background-color: " + backBright.name() + "; }\n";
    qss += "QPushButton:flat { background-color: transparent; border: none;}\n";
    app.setStyleSheet(qss);
}


void addDarkTitlebar(QMainWindow* widget) {
    const int height = 30; // px
    widget->setWindowFlags(Qt::FramelessWindowHint);
    auto size = widget->size();
    widget->resize(size.width(), size.height() + height);

    for(auto o : widget->centralWidget()->children()) {
        if (!o->isWidgetType()) continue;
        auto& c = *static_cast<QWidget*>(o);
        std::cout << "move " << c.objectName().toStdString() << std::endl;
        c.move(c.pos() + QPoint{0, height});
    }
}




