#ifndef PRG_LOADER_H
#define PRG_LOADER_H

#include <QDialog>

#include "c64_runner.h"

namespace Ui {
class PRGLoader;
}

class PRGLoader : public QDialog
{
    Q_OBJECT

public:
    explicit PRGLoader(QWidget *parent, C64Runner* c64Runner, const std::string& fileName);
    ~PRGLoader();
    static void openPRGFile(QWidget *parent, C64Runner* c64Runner);

private:
    Ui::PRGLoader *ui;
    C64Runner *c64Runner;
};

#endif // PRG_LOADER_H
