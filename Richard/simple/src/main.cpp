#include <QtWidgets/QApplication>
#include "mainwindow.h"
#include "glinc.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
