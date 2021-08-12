#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("CLSA");
    QCoreApplication::setOrganizationDomain("clsa-elcv.ca");
    QCoreApplication::setApplicationName("pine_masimo");

    // TODO: for debug mode command line arg, where no
    // GUI is required we can run a non-gui version
    //

    QApplication a(argc, argv);
    MainWindow w;
    w.setApplicationDir(a.applicationDirPath());
    w.show();
    return a.exec();
}
