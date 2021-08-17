#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QTextStream>

#include "CognitiveIO.h"
#include "ExitCodesEnum.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("CLSA");
    QApplication::setOrganizationDomain("clsa-elcv.ca");
    QApplication::setApplicationName("cypress_cognitive-test");

    // Read the command line arguments and create an inputs model
    InputsModel inputs;
    int exitCode = CognitiveIO::ParseCommandLineArgs(&app, &inputs);

    qDebug() << "Main.cpp:" <<  endl;
    qDebug() << "mode: " << inputs.mode << endl;
    qDebug() << "json inputs: " + inputs.jsonInputsPath << endl;
    qDebug() << "json output path: " + inputs.jsonOutputsPath << endl;
    qDebug() << "json user id: " + inputs.userID << endl;
    qDebug() << "json site name: " + inputs.dcsSiteName << endl;
    qDebug() << "json interviewer id: " + inputs.interviewerID << endl;
    qDebug() << "json language: " + inputs.language << endl;
    
    // If command line arguments are invalid, return with the corresponding exitcode
    if (exitCode != ExitCodes::Continue) {
        return exitCode;
    }

    // At this point command line arguments have been validated and settings read in
    // Exit with succesful code if the app is running in ghost mode
    if (inputs.mode == Modes::ghost) return ExitCodes::Successful;

    // Start App
    CognitiveTest w(inputs);
    w.show();
    return app.exec();
}
