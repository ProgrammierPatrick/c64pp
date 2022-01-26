#pragma once

#include <QMainWindow>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>

#include "c64_runner.h"

namespace Ui {
class RAMViewer;
}
class QTableView;

class MainWindow;

class RAMViewer;
class RAMViewerModel : public QAbstractTableModel {
    Q_OBJECT

public:
    RAMViewerModel(RAMViewer* viewer, C64Runner *c64Runner)
        : viewer(viewer), c64Runner(c64Runner) { }

    void updateC64();

    int columnCount(const QModelIndex& index = QModelIndex()) const override;
    int rowCount(const QModelIndex& index = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    RAMViewer* viewer;
    C64Runner* c64Runner;

    int ramSelection = 0;
    bool relativeOffset = false;
    bool showDecimalAddr = false;
};

class RAMViewerItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QTableView* view;
};


class RAMViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit RAMViewer(MainWindow *parent, C64Runner* c64Runner);
    ~RAMViewer();

    void updateC64();

private:
    Ui::RAMViewer *ui;
    C64Runner *c64Runner;
    MainWindow *mainWindow;

    RAMViewerModel model;
    RAMViewerItemDelegate itemDelegate;
};

