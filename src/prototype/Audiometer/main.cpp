#include "MainWindow.h"

#include <QApplication>
#include <QLocale>
#include <QMessageBox>
#include <QTranslator>
#include "../../auxiliary/CommandLineParser.h"
#include "../../auxiliary/Utilities.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("CLSA");
    QCoreApplication::setOrganizationDomain("clsa-elcv.ca");
    QCoreApplication::setApplicationName("Audiometer");
    QCoreApplication::setApplicationVersion("1.0.0");

    QApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Audiometer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    // start logging
    // for development purposes, store logging in the working directory
    //
    QString logfile = QCoreApplication::applicationDirPath() + QStringLiteral("/") + "log.txt";
    Logging::useFile(logfile);

    // process command line args
    //
    CommandLineParser parser;
    QString errMessage;
    switch(parser.parseCommandLine(app,&errMessage))
    {
      case CommandLineParser::parseHelpRequested:
        QMessageBox::warning(0, QGuiApplication::applicationDisplayName(),
                                 "<html><head/><body><pre>"
                                 + parser.helpText() + "</pre></body></html>");
        return 0;
      case  CommandLineParser::parseVersionRequested:
        QMessageBox::information(0, QGuiApplication::applicationDisplayName(),
                                 QGuiApplication::applicationDisplayName() + ' '
                                 + QCoreApplication::applicationVersion());
        return 0;
      case CommandLineParser::parseOk:
        break;
      case CommandLineParser::parseError:
      case CommandLineParser::parseInputFileError:
      case CommandLineParser::parseOutputPathError:
      case CommandLineParser::parseMissingArg:
      case CommandLineParser::parseMeasureTypeError:
      case CommandLineParser::parseRunModeError:
        QMessageBox::warning(0, QGuiApplication::applicationDisplayName(),
                             "<html><head/><body><h2>" + errMessage + "</h2><pre>"
                             + parser.helpText() + "</pre></body></html>");
        return 1;
    }

    MainWindow window;
    QMap<QString,QVariant> args = parser.getArgs();
    window.setInputFileName(args["inputFileName"].toString());
    window.setOutputFileName(args["outputFileName"].toString());
    window.setRunMode(args["runMode"].value<Constants::RunMode>());
    window.setVerbose(args["verbose"].toBool());

    window.show();
    window.initialize();

    return app.exec();
}
