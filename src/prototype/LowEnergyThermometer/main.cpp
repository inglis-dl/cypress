#include "MainWindow.h"

#include <QApplication>
#include "../../auxiliary/CommandLineParser.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("CLSA");
    QCoreApplication::setOrganizationDomain("clsa-elcv.ca");
    QCoreApplication::setApplicationName("LowEnergyThermometer");

    QApplication app(argc, argv);

    // process command line args
    // TODO: if the run mode arg is 'simulate'
    // do not open the window, just write dummy output data to json
    //
    CommandLineParser parser;
    parser.parseCommandLine(app);

    MainWindow window;
    window.setInputFileName(parser.getInputFilename());
    window.setOutputFileName(parser.getOutputFilename());
    window.setMode(parser.getRunMode());
    window.setVerbose(true/*verbose*/);

    window.initialize();
    window.show();
    window.run();

    return app.exec();
}
