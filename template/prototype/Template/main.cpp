#include "MainWindow.h"

#include <QApplication>
#include <QLocale>
#include <QMessageBox>
#include <QTranslator>
#include "../../../src/auxiliary/CommandLineParser.h"

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("CLSA");
    QApplication::setOrganizationDomain("clsa-elcv.ca");
    QApplication::setApplicationName("TEMPLATE");
    QApplication::setApplicationVersion("1.0.0");

    QApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "TEMPLATE_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

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
    QVariantMap args = parser.getArgs();
    window.setInputFileName(args["inputFileName"].toString());
    window.setOutputFileName(args["outputFileName"].toString());
    window.setRunMode(args["runMode"].value<Constants::RunMode>());
    window.setVerbose(args["verbose"].toBool());

    window.show();
    window.initialize();

    return app.exec();
}
