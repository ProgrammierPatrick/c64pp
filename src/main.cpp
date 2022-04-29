#include "gui/main_window.h"
#include "gui/style.h"

#include "tools/parse_args.h"

#include <QApplication>

#include <string>
#include <vector>

int runWindow(int argc, char** argv) {
    QApplication app(argc, argv);
    applyStyle(app);

    MainWindow win;
    win.show();

    return app.exec();
}

int main(int argc, char** argv) {
    if (argc > 1) {
        if (std::string(argv[1]).ends_with(".prg") || std::string(argv[1]).ends_with(".PRG")) {
            QApplication app(argc, argv);
            MainWindow win;
            win.show();
            win.tickFrames(120);
            win.loadPRG(argv[1]);
            return app.exec();
        }

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
        return runWindow(argc, argv);
    }
}
