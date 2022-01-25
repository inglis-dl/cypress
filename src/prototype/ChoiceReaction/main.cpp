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
    QApplication::setApplicationName("ChoiceReaction");
    QApplication::setApplicationVersion("1.0.0");

    QApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ChoiceReaction_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    // Need to pull changes from development branch to get all features of commandLineParser
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

    return app.exec();
}
