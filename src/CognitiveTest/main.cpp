#include "MainWindow.h"
#include <QtWidgets/QApplication>

#include "CognitiveIO.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    InputsModel commandLineInputs = CognitiveIO::ReadCommandLineInputs(argc, argv);

    CognitiveTest w(commandLineInputs);
    w.show();
    return a.exec();
}
