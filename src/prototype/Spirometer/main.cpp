#include "Spirometer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Spirometer w;
    w.show();
    return a.exec();
}
