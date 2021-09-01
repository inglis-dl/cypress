#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("CLSA");
    QCoreApplication::setOrganizationDomain("clsa-elcv.ca");
    QCoreApplication::setApplicationName("Cypress");
    QCoreApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "cypress_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    

    // TODO
    // - command line parsing
    // expect input json defining which instrument to run
    // 

    // read in any .ini file settings
    //


    // - logging categories
    // - singleton ?
    // - error codes
    // - dialog classes
    // - default .ini settings
    // - test data

    return app.exec();
}
