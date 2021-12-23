#include "CypressApplication.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("CLSA");
    QCoreApplication::setOrganizationDomain("clsa-elcv.ca");
    QCoreApplication::setApplicationName("WeighScale");
    QCoreApplication::setApplicationVersion("1.0.0");

    QApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "weighscale_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    // process command line args
    //
    CypressApplication cypress;
    QString errMessage;
    switch(cypress.parse(app,&errMessage))
    {
      case CypressApplication::CommandLineHelpRequested:
        QMessageBox::warning(0, QGuiApplication::applicationDisplayName(),
                                 "<html><head/><body><pre>"
                                 + cypress.helpText() + "</pre></body></html>");
        return 0;
      case  CypressApplication::CommandLineVersionRequested:
        QMessageBox::information(0, QGuiApplication::applicationDisplayName(),
                                 QGuiApplication::applicationDisplayName() + ' '
                                 + QCoreApplication::applicationVersion());
        return 0;
      case CypressApplication::CommandLineOk:
        break;
      case CypressApplication::CommandLineError:
      case CypressApplication::CommandLineInputFileError:
      case CypressApplication::CommandLineOutputPathError:
      case CypressApplication::CommandLineMissingArg:
      case CypressApplication::CommandLineTestTypeError:
      case CypressApplication::CommandLineModeError:
        QMessageBox::warning(0, QGuiApplication::applicationDisplayName(),
                             "<html><head/><body><h2>" + errMessage + "</h2><pre>"
                             + cypress.helpText() + "</pre></body></html>");
        return 1;
    }

    try
    {
      qDebug() << "initialize start";
      cypress.initialize();
      qDebug() << "initialize end";

    }
    catch (std::exception& e)
    {
      qDebug() << e.what();
      QMessageBox::critical(0, QGuiApplication::applicationDisplayName(),
                           "<html><head/><body><h2>" + QString(e.what()) + "</h2></body></html>");
      return 0;
    }
    cypress.show();
    cypress.run();

    return app.exec();
}
