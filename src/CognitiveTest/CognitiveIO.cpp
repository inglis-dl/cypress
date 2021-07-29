#include "CognitiveIO.h"

InputsModel CognitiveIO::ReadInputs(QString path)
{
    // Read in contents of json file
    QFile file(path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString val = file.readAll();
    file.close();

    // Create json object
    QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();

    return CreateInputsModel(&jsonObj);
}

InputsModel CognitiveIO::CreateInputsModel(QJsonObject* json)
{
    InputsModel inputs;
    inputs.path = json->value(QString("CCBFolderPath")).toString();
    inputs.userName = json->value(QString("UserName")).toString();
    inputs.dcsSiteName = json->value(QString("DcsSiteName")).toString();
    inputs.interviewId = json->value(QString("InterviewId")).toString();
    inputs.language = json->value(QString("Language")).toString();
    return inputs;
}
