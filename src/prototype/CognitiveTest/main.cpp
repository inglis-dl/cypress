#include "MainWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("CLSA");
    QApplication::setOrganizationDomain("clsa-elcv.ca");
    QApplication::setApplicationName("CognitiveTest");
    QApplication::setApplicationVersion("1.0.0");

    QApplication app(argc, argv);

    //TODO: Add commandLineParser from auxiliary

    // Start App
    CognitiveTest w;
    w.setInputFileName("C:/Users/clsa/Documents/SampleJsonFiles/CognitiveTestInput.json");
    w.setOutputFileName("C:/Users/clsa/Documents/SampleJsonFiles");
    w.setMode("live");
    w.setVerbose(true);

    w.initialize();
    w.show();
    w.run();
    return app.exec();
}
