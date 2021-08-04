#include "CognitiveIO.h"

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

    inputs->path = jsonObj.value(QString("CCBFolderPath")).toString();
}

InputsModel CognitiveIO::ReadCommandLineInputs(int argc, char* argv[])
{
    InputsModel commandLineInputs;

    // Keep track of argument number
    // This does not increment if -test is found
    int argNum = 0;

    // Track any unexpected arguments entered
    int unexpectedArgs = 0;

    // Loop over command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg1(argv[i]);
        QString argument = QString::fromStdString(arg1);

        if (argument.startsWith("-")) {
            if (argument == "-test") {
                commandLineInputs.isTest = true;
            }
        }
        else {
            AddArgToInputs(argNum, &unexpectedArgs, &commandLineInputs, &argument);
            argNum++;
        }
    }

    return commandLineInputs;
}

void CognitiveIO::AddArgToInputs(int argNum, int* unexpectedArgs, InputsModel* inputs, QString* argument) {
    switch (argNum) {
    case 0:
        inputs->userID = *argument;
        break;
    case 1:
        inputs->dcsSiteName = *argument;
        break;
    case 2:
        inputs->interviewerID = *argument;
        break;
    case 3:
        if (argument->toLower() == "en") {
            inputs->language = "English";
        }
        else if (argument->toLower() == "fr") {
            inputs->language = "French";
        }
        break;
    default:
        *unexpectedArgs++;
    }
}
