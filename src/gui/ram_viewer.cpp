#include "ram_viewer.h"
#include "ui_ram_viewer.h"

#include "main_window.h"
#include "text_utils.h"

#include <QAbstractTableModel>
#include <QLineEdit>

RAMViewer::RAMViewer(MainWindow *parent, C64Runner *c64Runner) :
    QMainWindow(parent),
    ui(new Ui::RAMViewer),
    c64Runner(c64Runner), mainWindow(parent), model(this, c64Runner)
{
    ui->setupUi(this);

    setFixedSize(size());

    itemDelegate.view = ui->tableView;

    ui->tableView->setModel(&model);
    ui->tableView->setItemDelegate(&itemDelegate);

    QObject::connect(ui->relAddrCheckBox, &QCheckBox::toggled, [this](bool checked) {
        model.relativeOffset = checked;
        updateC64();
    });
    QObject::connect(ui->decimalAddrCheckBox, &QCheckBox::toggled, [this](bool checked) {
        model.showDecimalAddr = checked;
        updateC64();
    });
    QObject::connect(ui->whichMemoryComboBox, &QComboBox::currentIndexChanged, [this](int index) {
        model.ramSelection = index;
        updateC64();
    });

    updateC64();
}

RAMViewer::~RAMViewer()
{
    delete ui;
}

void RAMViewer::updateC64() {
    model.updateC64();
}

int ramSize(int index) {
    switch(index) {
    case 0: return 0x10000; // MPUMemoryView
    case 1: return 0x10000; // Main RAM
    case 2: return 1024;    // Color RAM
    case 3: return 0x2000;  // BASIC ROM
    case 4: return 0x1000;  // chargen ROM
    case 5: return 0x1000;  // KERNAL ROM
    case 6: return 0x40;    // VIC-II
    case 7: return 16;      // CIA 1+2
    default: throw std::runtime_error("RamViewer: illegal memory type selected");
    }
}

uint16_t getOffset(int index) {
    switch(index) {
    case 0: return 0;
    case 1: return 0;
    case 2: return 0xD800;
    case 3: return 0xA000;
    case 4: return 0xD000;
    case 5: return 0xE000;
    case 6: return 0xD000;
    case 7: return 0xDC00;
    default: throw std::runtime_error("RamViewer: illegal memory type selected");
    }
}

uint8_t readMem(C64& c64, int index, uint16_t addr) {
    switch(index) {
    case 0: return c64.mpuMemoryView.read(addr, true);
    case 1: return c64.mainRAM.read(addr, true);
    case 2: return c64.colorRAM.read(addr, true);
    case 3: return c64.basicROM.read(addr, true);
    case 4: return c64.chargenROM.read(addr, true);
    case 5: return c64.kernalROM.read(addr, true);
    case 6: return c64.vic.read(addr, true);
    case 7: return c64.cia.read(addr, true);
    default: throw std::runtime_error("RamViewer: illegal memory type selected");
    }
}

void writeMem(C64& c64, int index, uint16_t addr, uint8_t val) {
    switch(index) {
    case 0: c64.mpuMemoryView.write(addr, val); break;
    case 1: c64.mainRAM.write(addr, val);       break;
    case 2: c64.colorRAM.write(addr, val);      break;
    case 3: c64.basicROM.write(addr, val);      break;
    case 4: c64.chargenROM.write(addr, val);    break;
    case 5: c64.kernalROM.write(addr, val);     break;
    case 6: c64.vic.write(addr, val);           break;
    case 7: c64.cia.write(addr, val);           break;
    default: throw std::runtime_error("RamViewer: illegal memory type selected");
    }
}

bool isEditable(int index, uint16_t addr) {
    switch(index) {
    case 0: return true;  // MPUMemoryView
    case 1: return true;  // Main RAM
    case 2: return true;  // Color RAM
    case 3: return false; // BASIC ROM
    case 4: return false; // chargen ROM
    case 5: return false; // KERNAL ROM
    case 6: return true;  // VIC-II
    case 7: return true;  // CIA 1+2
    default: throw std::runtime_error("RamViewer: illegal memory type selected");
    }
}


bool ramviewer_isupdating = true;
void RAMViewerModel::updateC64() {
    if (ramviewer_isupdating) {
        emit dataChanged(index(0, 0), index(ramSize(ramSelection) / 16, 15));
        emit headerDataChanged(Qt::Orientation::Vertical, 0, ramSize(ramSelection) / 16);
    }
}

// https://doc.qt.io/qt-6.2/model-view-programming.html#creating-new-models
QVariant RAMViewerModel::data(const QModelIndex& index, int role) const {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return QString::fromStdString(toHexStr(readMem(*c64Runner->c64, ramSelection, index.row() * 16 + index.column())));
    else
        return QVariant();
}

QVariant RAMViewerModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Vertical) {
            if (showDecimalAddr)
                return QString::fromStdString(std::to_string(relativeOffset ? (section * 16) : (section * 16 + getOffset(ramSelection))));
            else
                return QString::fromStdString(toHexStr(static_cast<uint16_t>(relativeOffset ? (section * 16) : (section * 16 + getOffset(ramSelection)))));
        }
    }
    return QVariant();
}

int RAMViewerModel::rowCount(const QModelIndex& index) const {
    return ramSize(ramSelection) / 16;
}

int RAMViewerModel::columnCount(const QModelIndex& index) const {
    return 16;
}

Qt::ItemFlags RAMViewerModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if (isEditable(ramSelection, index.row() * 16 + index.column()))
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    else return QAbstractItemModel::flags(index);
}

bool RAMViewerModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole) {
        uint16_t addr = index.row() * 16 + index.column();
        uint8_t data = fromHexStr8(value.toString().toStdString());
        writeMem(*c64Runner->c64, ramSelection, addr, data);
        return true;
    }
    return false;
}


QWidget *RAMViewerItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    ramviewer_isupdating = false;
    QWidget *editor = QStyledItemDelegate::createEditor(parent, option, index);

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
    QObject::connect(lineEdit, &QLineEdit::cursorPositionChanged, [lineEdit](int oldpos, int newpos) {
        if (newpos == 2) {
            lineEdit->clearFocus();
        } });
    QObject::connect(lineEdit, &QObject::destroyed, [](QObject *o) { ramviewer_isupdating = true; });
    return lineEdit;
}
