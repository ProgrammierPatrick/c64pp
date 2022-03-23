#include "gui/main_window.h"

#include "tools/parse_args.h"

#include <QApplication>

#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc > 1) {
        if (argv[1] == std::string("args")) {
            std::vector<char*> args(argv, argv + argc);
            args.erase(args.begin() + 1);
            int size = args.size();
            QApplication app(size, args.data());
            MainWindow win;
            win.show();
            return app.exec();
        } else {
            return parseArgs(argc, argv);
        }
    } else {
        QApplication app(argc, argv);
        MainWindow win;
        win.show();
        return app.exec();
    }
}
