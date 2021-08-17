#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("CLSA");
    QCoreApplication::setOrganizationDomain("clsa-elcv.ca");
    QCoreApplication::setApplicationName("pine_masimo");

    // TODO: for debug mode command line arg, where no
    // GUI is required we can run a non-gui version
    //

    QApplication app(argc, argv);

    // process command line args
    // if the run mode arg is 'simulate'
    // do not open the window, just write dummy output data to json
    //
    QCommandLineParser parser;

    // expect a full path to an input.json file
    // which contains the minimum necessary input data to
    // run the device (eg., barcode, gender etc.)

    QCommandLineOption inputOption(
      QStringList() << "i" << "input",
      QCoreApplication::translate(
        "main", "Full path spec to input json file"));
    parser.addOption(inputOption);
    QCommandLineOption outputOption(
      QStringList() << "o" << "output",
      QCoreApplication::translate(
        "main", "Full path spec to output json file"));
    parser.addOption(outputOption);
    QCommandLineOption modeOption(
      QStringList() << "m" << "mode",
      QCoreApplication::translate(
        "main", "Run mode <live,ghost,debug>"));
    modeOption.setDefaultValue("debug");
    parser.addOption(modeOption);
    parser.process(app);

    QString inputFilename;
    if(parser.isSet(inputOption))
    {
        inputFilename = parser.value(inputOption);
    }
    QString outputFilename;
    if(parser.isSet(outputOption))
    {
        outputFilename = parser.value(outputOption);
    }
    QString mode;
    if(parser.isSet(mode))
    {
        mode = parser.value(modeOption);
    }

    MainWindow window;
    window.setInputFileName(inputFilename);
    window.setOutputFileName(outputFilename);
    window.setMode(mode);
    window.show();
    window.run();

    return app.exec();
}
