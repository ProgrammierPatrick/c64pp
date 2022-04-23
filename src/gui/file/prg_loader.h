#ifndef PRG_LOADER_H
#define PRG_LOADER_H

#include <QDialog>

#include "../c64_runner.h"

namespace Ui {
class PRGLoader;
}

class MainWindow;

class PRGLoader : public QDialog
{
    Q_OBJECT

public:
    explicit PRGLoader(MainWindow *parent, C64Runner* c64Runner, const std::string& fileName, bool fastAccept = false);
    ~PRGLoader();
    static void openPRGFile(MainWindow *parent, C64Runner* c64Runner);

private:
    Ui::PRGLoader *ui;
    C64Runner *c64Runner;
    MainWindow *mainWindow;
};

#endif // PRG_LOADER_H
