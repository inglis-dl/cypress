#include "BPTruBPM200.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BPTruBPM200 w;
    w.show();
    return a.exec();
}
