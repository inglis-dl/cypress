#include "CypressApplication.h"

#include "./auxiliary/CommandLineParser.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>
#include <QDebug>

#include "./auxiliary/Constants.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("CLSA");
    QCoreApplication::setOrganizationDomain("clsa-elcv.ca");
    QCoreApplication::setApplicationName("Cypress");
    QCoreApplication::setApplicationVersion("1.0.0");

    QApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "cypress_" + QLocale(locale).name();
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
    case CommandLineParser::VersionRequested:
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
    return EXIT_SUCCESS;
  }

  CypressApplication cypress;
  try
  {
      qDebug() << "initialize start";
      cypress.setArgs(parser.getArgs());
      cypress.initialize();
      qDebug() << "initialize end";
  }
  catch (std::exception& e)
  {
      qDebug() << e.what();
      QMessageBox::critical(0, QGuiApplication::applicationDisplayName(),
                           "<html><head/><body><h2>" + QString(e.what()) + "</h2></body></html>");
      return EXIT_FAILURE;
  }

  return app.exec();
}
