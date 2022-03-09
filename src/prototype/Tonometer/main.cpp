#include "MainWindow.h"

#include <QApplication>
#include <QLocale>
#include <QMessageBox>
#include <QTranslator>
#include "../../auxiliary/CommandLineParser.h"

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("CLSA");
    QApplication::setOrganizationDomain("clsa-elcv.ca");
    QApplication::setApplicationName("Tonometer");
    QApplication::setApplicationVersion("1.0.0");

    QApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Tonometer_" + QLocale(locale).name();
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
      case CommandLineParser::HelpRequested:
        QMessageBox::warning(0, QGuiApplication::applicationDisplayName(),
                                 "<html><head/><body><pre>"
                                 + parser.helpText() + "</pre></body></html>");
        return 0;
      case  CommandLineParser::VersionRequested:
        QMessageBox::information(0, QGuiApplication::applicationDisplayName(),
                                 QGuiApplication::applicationDisplayName() + ' '
                                 + QCoreApplication::applicationVersion());
        return 0;
      case CommandLineParser::Ok:
        break;
      case CommandLineParser::Error:
      case CommandLineParser::InputFileError:
      case CommandLineParser::OutputPathError:
      case CommandLineParser::MissingArg:
      case CommandLineParser::MeasureTypeError:
      case CommandLineParser::RunModeError:
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
