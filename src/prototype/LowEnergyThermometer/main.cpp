#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("CLSA");
    QCoreApplication::setOrganizationDomain("clsa-elcv.ca");
    QCoreApplication::setApplicationName("LowEnergyThermometer");

    // TODO: for debug mode command line arg, where no
    // GUI is required we can run a non-gui version
    //

    QApplication app(argc, argv);

    // process command line args
    // if the run mode arg is 'simulate'
    // do not open the window, just write dummy output data to json
    //
    QCommandLineParser parser;
    parser.addHelpOption();

    // expect a full path to an input.json file
    // which contains the minimum necessary input data to
    // run the device (eg., barcode, gender etc.)

    QCommandLineOption inputOption(
      QStringList() << "i" << "input",
      QCoreApplication::translate(
        "main", "Read json input from <file>"),"file");
    parser.addOption(inputOption);
    QCommandLineOption outputOption(
      QStringList() << "o" << "output",
      QCoreApplication::translate(
        "main", "Write json output to <file>"),"file");
    parser.addOption(outputOption);
    QCommandLineOption modeOption(
      QStringList() << "m" << "mode",
      QCoreApplication::translate(
        "main", "Run mode <live,ghost,debug>"),"runMode","debug");
    parser.addOption(modeOption);
    QCommandLineOption verboseOption(
       QStringList() << "v" << "verbose",
       QCoreApplication::translate(
         "main","Verbose mode. Prints out verbose debug information."));
    parser.addOption(verboseOption);
    parser.process(app);

    // Default when not run from command line we assume verbose is desired
    //
    bool verbose = parser.isSet(verboseOption);

    QString inputFilename;
    if(parser.isSet(inputOption))
    {
        inputFilename = parser.value(inputOption);
        if(verbose)
          qDebug() << "in file option set with " << inputFilename;
    }
    QString outputFilename;
    if(parser.isSet(outputOption))
    {
        outputFilename = parser.value(outputOption);
        if(verbose)
          qDebug() << "out file option set with " << outputFilename;
    }
    QString mode;
    if(parser.isSet(modeOption))
    {
        mode = parser.value(modeOption);
        if(verbose)
          qDebug() << "mode option set with " << mode;
    }

    MainWindow window;
    window.setInputFileName(inputFilename);
    window.setOutputFileName(outputFilename);
    window.setMode(mode);
    window.setVerbose(true/*verbose*/);

    window.initialize();
    window.show();
    window.run();

    return app.exec();
}
