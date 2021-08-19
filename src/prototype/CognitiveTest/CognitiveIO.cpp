#include "CognitiveIO.h"

/*!
    Stores formatted command line argument to an InputsModel
    app: The application with the passed in arguments
    inputs: The inputs model to store inputs in
    RETURN: An exit code, either continue if there were no errors or 
            a specific error code if there were errors
*/
int CognitiveIO::ParseCommandLineArgs(QApplication* app, InputsModel* inputs)
{
    // Parse command line arguments and add them to inputs
    ParseCommandLineInputs(app, inputs);
    
    // Read values from json file located at json input path and add to inputs
    int exitCode = AddJsonInputs(inputs);
    if (exitCode != Continue) return exitCode;

    // Return appropriate exit code
    return DetermineExitCode(inputs);
}

void CognitiveIO::ParseCommandLineInputs(QApplication* app, InputsModel* inputs)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("QT app for launching Cognitive Test at CLSA Sites");
    parser.addHelpOption();

    QCommandLineOption modeOption("m",
        QApplication::translate("main", "Selects the mode to run the app in."),
        QApplication::translate("main", "The mode to run in (live, ghost or debug)."));
    parser.addOption(modeOption);

    QCommandLineOption inputPathOption("i",
        QApplication::translate("main", "Selects the input path."),
        QApplication::translate("main", "The input path."));
    parser.addOption(inputPathOption);

    QCommandLineOption outputPathOption("o",
        QApplication::translate("main", "Selects the output path."),
        QApplication::translate("main", "The output path."));
    parser.addOption(outputPathOption);

    parser.process(*app);

    // Set inputs collected from commandline
    inputs->mode = ModesHelper::DetermineMode(parser.value(modeOption));
    inputs->jsonInputsPath = parser.value(inputPathOption);
    inputs->jsonOutputsPath = parser.value(outputPathOption);
}

/*!
* Populate inputs model with the inputs located in the json file 
* pointed to by the json inputs file path
*/
int CognitiveIO::AddJsonInputs(InputsModel* inputs)
{
    try {
        // Read in contents of json file
        QFile file(inputs->jsonInputsPath);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString val = file.readAll();
        file.close();

        // Create json object
        QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();

        // Store inputs
        inputs->userID = jsonObj.value(QString(Strings::jsonArg_userID)).toString();
        inputs->dcsSiteName = jsonObj.value(QString(Strings::jsonArg_siteName)).toString();
        inputs->interviewerID = jsonObj.value(QString(Strings::jsonArg_interviewerID)).toString();

        QString languageCommandText = jsonObj.value(QString(Strings::jsonArg_language)).toString().toLower();
        if (languageCommandText == Strings::cmdInput_English) {
            inputs->language = Strings::English;
        }
        else if (languageCommandText == Strings::cmdInput_French) {
            inputs->language = Strings::French;
        }
    }
    catch(...){
        return ExitCodes::ProblemWithJsonInputsPath;
    }
    return ExitCodes::Continue;
}

/*!
    Determine the exit code to be reported by CreateInputsModelFromCmdArgs
    argNum: The total number of non-flag arguments passed to the command line
    test: True if this is a test, false otherwise
    fullTest: True if this is a full test, false otherwise
    RETURN: The appropriate exit code
*/
int CognitiveIO::DetermineExitCode(InputsModel* inputs) {
    qDebug() << "Determining exit code "<< endl;

    if (inputs->userID == "" || inputs->userID.length() != 8) {
        return ExitCodes::InvalidOrMissingUserId;
    }
    else if (inputs->dcsSiteName == "") {
        return ExitCodes::InvalidOrMissingSitename;
    }
    else if (inputs->interviewerID == "") {
        return ExitCodes::InvalidOrMissingInterviewerID;
    }
    else if (inputs->language == "") {
        return ExitCodes::InvalidOrMissingLanguage;
    }
    else {
        return ExitCodes::Continue;
    }
}
