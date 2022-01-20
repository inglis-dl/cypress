#include "MainWindow.h"

#include "../../../src/auxiliary/CommandLineParser.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("CLSA");
    QApplication::setOrganizationDomain("clsa-elcv.ca");
    QApplication::setApplicationName("TEMPLATE");
    QApplication::setApplicationVersion("1.0.0");

    QApplication app(argc, argv);

    CommandLineParser parser;
    QString errMessage;
    switch (parser.parseCommandLine(app, &errMessage))
    {
    case CommandLineParser::CommandLineHelpRequested:
        QMessageBox::warning(0, QGuiApplication::applicationDisplayName(),
            "<html><head/><body><pre>"
            + parser.helpText() + "</pre></body></html>");
        return 0;
    case  CommandLineParser::CommandLineVersionRequested:
        QMessageBox::information(0, QGuiApplication::applicationDisplayName(),
            QGuiApplication::applicationDisplayName() + ' '
            + QCoreApplication::applicationVersion());
        return 0;
    case CommandLineParser::CommandLineOk:
        break;
    case CommandLineParser::CommandLineError:
    case CommandLineParser::CommandLineInputFileError:
    case CommandLineParser::CommandLineOutputPathError:
    case CommandLineParser::CommandLineMissingArg:
    case CommandLineParser::CommandLineModeError:
        QMessageBox::warning(0, QGuiApplication::applicationDisplayName(),
            "<html><head/><body><h2>" + errMessage + "</h2><pre>"
            + parser.helpText() + "</pre></body></html>");
        return 1;
    }

    MainWindow window;
    window.setInputFileName(parser.getInputFilename());
    window.setOutputFileName(parser.getOutputFilename());
    window.setMode(parser.getMode());
    window.setVerbose(parser.getVerbose());

    window.show();
    window.initialize();
    window.run();

    return app.exec();
}
