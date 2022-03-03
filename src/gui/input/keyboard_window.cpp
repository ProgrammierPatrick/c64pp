#include "keyboard_window.h"
#include "ui_keyboard_window.h"

#include "../main_window.h"

/*
 * Keyboard size: 849 x 236
 * Matrix size: 221 x 231
 *
 * location with 10px gap:
 * Keyboard: 0, 0
 * Matrix: 859, 2
 *
 * Window sizes:
 * without matrix: 849 x 236
 * with matrix: 1080 x 236
 */

KeyboardWindow::KeyboardWindow(QWidget *parent, C64Runner *c64Runner, MainWindow *mainWindow) :
    QMainWindow(parent),
    ui(new Ui::KeyboardWindow),
    c64Runner(c64Runner), mainWindow(mainWindow)
{
    ui->setupUi(this);

    setFixedSize(size());

    keyboardWidget = new KeyboardWidget(centralWidget(), c64Runner, mainWindow);
    keyboardWidget->lower();

    QObject::connect(ui->showMatrixButton, &QPushButton::pressed, [this]() {
        if (showMatrix) {
            delete keyboardMatrix;
            showMatrix = false;
            this->resize(849, 236);
            ui->showMatrixButton->setText("show matrix");
        } else {
            keyboardMatrix = new KeyboardMatrixWidget(centralWidget(), this->c64Runner);
            showMatrix = true;
            this->resize(1080, 236);
            keyboardMatrix->move(859, 2);
            keyboardMatrix->raise();
            keyboardMatrix->show();
            ui->showMatrixButton->setText("hide matrix");
        }
        updateUI();
    });
    ui->showMatrixButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    this->setWindowTitle("Virtual Keyboard");
}

KeyboardWindow::~KeyboardWindow()
{
    delete ui;
}

void KeyboardWindow::updateUI() {
    keyboardWidget->updateUI();
    if (showMatrix)
        keyboardMatrix->updateUI();
}

void KeyboardWindow::keyPressEvent(QKeyEvent* event) {
    c64Runner->keyboard->handleKeyPressEvent(event);
    mainWindow->updateUI();
}

void KeyboardWindow::keyReleaseEvent(QKeyEvent* event) {
    c64Runner->keyboard->handleKeyReleaseEvent(event);
    mainWindow->updateUI();
}
