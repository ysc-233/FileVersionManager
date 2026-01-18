#include "ui/mainwindow.h"

#include <QApplication>
#include "utils/logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;
    w.show();
    Logger::info("Application started");
    int ret = app.exec();

    Logger::info("Application exited normally");
    return ret;
}
