#include <QApplication>
#include <QCommandLineParser>
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument("path", "The directory to open");
    parser.process(a);

    QString initialPath;

    if (!parser.positionalArguments().isEmpty()) {
        initialPath = parser.positionalArguments().first();
    }

    MainWindow w;

    if (!initialPath.isEmpty()) {
        if (initialPath.startsWith("~")) {
            initialPath.replace(0, 1, QDir::homePath());
        }

        w.openPath(initialPath);
    }

    w.show();
    return a.exec();
}
