#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("CLSA");
    QCoreApplication::setOrganizationDomain("clsa-elcv.ca");
    QCoreApplication::setApplicationName("pine_masimo");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
