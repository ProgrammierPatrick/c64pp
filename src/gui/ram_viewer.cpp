#include "ram_viewer.h"
#include "ui_ram_viewer.h"

#include "main_window.h"
#include "text_utils.h"

#include <QAbstractTableModel>
#include <QLineEdit>

RAMViewer::RAMViewer(MainWindow *parent, C64Runner *c64Runner) :
    QMainWindow(parent),
    ui(new Ui::RAMViewer),
    mainWindow(parent),
    c64Runner(c64Runner), model(this, c64Runner)
{
    ui->setupUi(this);

    itemDelegate.view = ui->tableView;

    ui->tableView->setModel(&model);
    ui->tableView->setItemDelegate(&itemDelegate);

    updateC64();
}

RAMViewer::~RAMViewer()
{
    delete ui;
}

void RAMViewer::updateC64() {

}

// https://doc.qt.io/qt-6.2/model-view-programming.html#creating-new-models
QVariant RAMViewerModel::data(const QModelIndex& index, int role) const {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return QString::fromStdString(toHexStr(c64Runner->c64->mainRAM.read(index.row() * 16 + index.column(), true)));
    else
        return QVariant();
}

QVariant RAMViewerModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Vertical)
            return QString::fromStdString(toHexStr(static_cast<uint16_t>(section * 16)));
    }
    return QVariant();
}

int RAMViewerModel::rowCount(const QModelIndex& index) const {
    return 64 * 1024 / 16;
}

int RAMViewerModel::columnCount(const QModelIndex& index) const {
    return 16;
}

Qt::ItemFlags RAMViewerModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool RAMViewerModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole) {
        std::cout << "write: " << index.row() << "," << index.column() << ": " << value.toString().toStdString() << "\n";
        uint16_t addr = index.row() * 16 + index.column();
        uint8_t data = fromHexStr8(value.toString().toStdString());
        c64Runner->c64->mainRAM.write(addr, data);
        std::cout << "now at " << toHexStr(addr) << ": " << toHexStr(c64Runner->c64->mainRAM.read(addr, true)) << std::endl;
        return true;
    }
    return false;
}


QWidget *RAMViewerItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QWidget *editor = QStyledItemDelegate::createEditor(parent, option, index);
    // std::cout << "Editor type: " << editor->metaObject()->className() << std::endl;

    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setMaxLength(2);
    lineEdit->setInputMask("HH");
    QObject::connect(lineEdit, &QLineEdit::editingFinished, [this]() {
        auto curr = view->selectionModel()->currentIndex();
        int nextX = (curr.column() + 1) % 16;
        int nextY = curr.row() + (nextX == 0 ? 1 : 0);
        if (nextY >= view->model()->rowCount()) return;
        QModelIndex index = view->model()->index(nextY, nextX);
        view->setCurrentIndex(index);
        view->setFocus();
    });
    QObject::connect(lineEdit, &QLineEdit::cursorPositionChanged, [lineEdit, this](int oldpos, int newpos) {
        if (newpos == 2) {
            lineEdit->clearFocus();
        } });
    return lineEdit;
}
