#include "MainWindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "../../auxiliary/CommandLineParser.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("CLSA");
    QCoreApplication::setOrganizationDomain("clsa-elcv.ca");
    QCoreApplication::setApplicationName("Audiometer");

    QApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "audiometer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

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
    window.setVerbose(true/*parser.getVerbose()*/);
    qDebug() << "ok1";

    window.initialize();
    qDebug() << "ok2";
    window.show();
    qDebug() << "ok3";
    window.run();
    qDebug() << "ok4";

    return app.exec();
}
