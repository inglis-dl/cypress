#include "CognitiveIO.h"

/*! 
    Read settings stored in local json file into inputs model
    path: A path to the json file to be read
    inputs: The inputs model to load settings into
*/
void CognitiveIO::ReadJsonInputs(QString path, InputsModel* inputs)
{
    // Read in contents of json file
    QFile file(path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString val = file.readAll();
    file.close();

    // Create json object
    QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();

    // Store path to CCB.exe
    inputs->path = jsonObj.value(QString(Strings::jsonArg_ccbFolderPath)).toString();
}

/*!
    Stores formatted command line argument to an InputsModel
    argc: The number of args stored in argv
    argv[]: A list of command line arguments passed in
    commandLineInputs: The inputs model to store commandline settings in
    RETURN: An exit code, either continue if there were no errors or 
            a specific error code if there were errors
*/
int CognitiveIO::CreateInputsModelFromCmdArgs(int argc, char* argv[], InputsModel* commandLineInputs)
{
    // Keep track of argument number and do not incrment when flag found
    // This means flags can be anywhere in the list of command line arguments
    // The non-flag arguments must be in a specific order
    int argNum = 0;

    bool testFlagFound = false;
    bool fullTestFlagFound = false;

    // Loop over command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg1(argv[i]);
        QString argument = QString::fromStdString(arg1);

        // If flag...
        if (argument.startsWith("-")) {
            // Check for test flag or fulltest flag and ignore any other flags
            if (argument == Strings::cmdFlag_test) testFlagFound = true;
            if (argument == Strings::cmdFlag_test) fullTestFlagFound = true;
        }
        // Otherwise, non-flag argument
        else {
            // Add argument to commandLineInputs and exit if invalid argument
            int exitCode = AddArgToInputs(argNum, commandLineInputs, &argument);
            if (exitCode != ExitCodes::Continue) return exitCode;
            argNum++;
        }
    }

    // Return appropriate exit code
    return DetermineExitCode(argNum, testFlagFound, fullTestFlagFound);
}

/*!
    Determine the exit code to be reported by CreateInputsModelFromCmdArgs
    argNum: The total number of non-flag arguments passed to the command line
    test: True if this is a test, false otherwise
    fullTest: True if this is a full test, false otherwise
    RETURN: The appropriate exit code
*/
int CognitiveIO::DetermineExitCode(int argNum, bool test, bool fullTest) {
    // Return too few arguments if their are less than 4 arguments
    // This check is made at the end so that flags are not included in the argument count
    if (argNum < 4) return ExitCodes::TooFewArguments;
    // Return full test code if this is a full test
    else if (fullTest) return ExitCodes::FullTest;
    // Return test code if this is a test
    else if (test)return ExitCodes::Test;
    // Otherwise Continue as normal
    else return ExitCodes::Continue;
}

/*!
    Add an argument to the InputsModel based on the order the argument was 
    passed in to the command line
    argNum: The argument number, 0 - first argument, 4 - last argument passed in
    inputs: The InputsModel to add argument information to
    argument: The argument to be used
    RETURN: An exit code, either continue if there were no errors or 
            a specific error code if there were errors
*/
int CognitiveIO::AddArgToInputs(int argNum, InputsModel* inputs, QString* argument) {
    switch (argNum) {
    // User ID
    case 0:
        if (ValidateUserIDCmdArg(*argument) == false) return ExitCodes::InvalidUserId;
        inputs->userID = *argument;
        break;
    // DCS Site Name
    case 1:
        inputs->dcsSiteName = *argument;
        break;
    // Interviewer ID
    case 2:
        inputs->interviewerID = *argument;
        break;
    // Language
    case 3:
        if (ValidateLanguageCmdArg(*argument) == false) return ExitCodes::InvalidLanguage;
        
        // Format language for CCB.exe (either english or french)
        // Validation already passed to ensure language is either english or french
        if (argument->toLower() == Strings::cmdInput_French) {
            inputs->language = Strings::French;
        }
        else{
            inputs->language = Strings::English;
        }
        break;
    // Extra Unexpected Arguments
    default:
        return ExitCodes::TooManyArguments;
    }

    // Continue if no errors detected
    return ExitCodes::Continue;
}

/*!
*   Validate the user id command line argument
*   userID: The user id to validate
*   RETURN: True if the user id is valid, false otherwise
*/
bool CognitiveIO::ValidateUserIDCmdArg(QString userID) {
    return (userID).length() == 8;
}

/*!
*   Validate the site name command line argument
*   siteName: The site name to validate
*   RETURN: True if the site name is valid, false otherwise
*/
bool CognitiveIO::ValidateSiteNameCmdArg(QString siteName) {
    return true;
}

/*!
*   Validate the interviewer ID command line argument
*   interviewerID: The interviewer ID to validate
*   RETURN: True if the interviewer ID is valid, false otherwise
*/
bool CognitiveIO::ValidateInterviewerIDCmdArg(QString interviewerID) {
    return true;
}

/*!
*   Validate the language command line argument
*   language: The language to validate
*   RETURN: True if the language is valid, false otherwise
*/
bool CognitiveIO::ValidateLanguageCmdArg(QString language) {
    return language == Strings::cmdInput_English || language == Strings::cmdInput_French;
}