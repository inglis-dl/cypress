#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QTextStream>

#include "CognitiveIO.h"
#include "ExitCodesEnum.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Read the command line arguments and create an inputs model
    InputsModel commandLineInputs;
    int argumentsValidExitCode = CognitiveIO::CreateInputsModelFromCmdArgs(argc, argv, &commandLineInputs);
    
    // If command line arguments are invalid or this is a test, 
    // then exit the app with the corresponding exit code (Do not start app)
    if (argumentsValidExitCode != ExitCodes::Continue && argumentsValidExitCode != ExitCodes::FullTest) {
        return argumentsValidExitCode;
    }

    // Start App
    CognitiveTest w(commandLineInputs);

    // If something goes wrong during setup, exit with the proper error code (Do not start app)
    // TODO: Think about displaying a UI letting the user know there was a problem during setup
    int setupExitCode = w.GetExitCode();
    if (setupExitCode != ExitCodes::Continue) return setupExitCode;

    // At this point command line arguments have been validated and settings read in
    // Exit with succesful code if this is a full test (Do not start app)
    if (argumentsValidExitCode == ExitCodes::FullTest) return ExitCodes::Successful;

    // Start the app
    w.show();
    return a.exec();
}
