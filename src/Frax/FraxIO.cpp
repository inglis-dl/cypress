#include "FraxIO.h"

InputsModel FraxIO::ReadInputs(QString readPath)
{
    // Read in contents of json file
    QFile file(readPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString val = file.readAll();
    file.close();

    // Create json object
    QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();

    // Create inputs model from json object and return
    InputsModel inputsFromJson = CreateInputsModel(&jsonObj);
    return inputsFromJson;
}

InputsModel FraxIO::CreateInputsModel(QJsonObject* json) 
{
    InputsModel inputs;
    inputs.fraxFolderPath = json->value(QString("fraxFolderPath")).toString();
    inputs.val1 = json->value(QString("val1")).toString();
    inputs.val2 = json->value(QString("val2")).toDouble(); 
    inputs.val3 = json->value(QString("val3")).toDouble();
    inputs.val4 = json->value(QString("val4")).toDouble();
    inputs.val5 = json->value(QString("val5")).toDouble();
    inputs.val6 = json->value(QString("val6")).toDouble();
    inputs.val7 = json->value(QString("val7")).toDouble();
    inputs.val8 = json->value(QString("val8")).toDouble();
    inputs.val9 = json->value(QString("val9")).toDouble();
    inputs.val10 = json->value(QString("val10")).toDouble();
    inputs.val11 = json->value(QString("val11")).toDouble();
    inputs.val12 = json->value(QString("val12")).toDouble();
    inputs.dxaHipTScore = json->value(QString("dxaHipTScore")).toDouble();
    return inputs;
}

bool FraxIO::CreateInputTxt(InputsModel* inputs)
{
    QString filePath = inputs->fraxFolderPath + "/input.txt";
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);

        stream << inputs->val1 + ","
            << inputs->val2 << ","
            << inputs->val3 << "," 
            << inputs->val4 << "," 
            << inputs->val5 << "," 
            << inputs->val6 << "," 
            << inputs->val7 << ","
            << inputs->val8 << "," 
            << inputs->val9 << "," 
            << inputs->val10 << "," 
            << inputs->val11 << "," 
            << inputs->val12 << "," 
            << inputs->dxaHipTScore;

        file.close();
        qDebug() << "Wrote input.txt to " + filePath;
        return true;
    }
    return false;
}

OutputsModel FraxIO::ReadOutputs(QString readPath)
{
    QFile file(readPath);

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream instream(&file);
        QString line = instream.readLine();
        file.close();

        OutputsModel outputs;
        QStringList lineSplit = line.split(",");
        if (lineSplit.length() >= 17) {
            outputs.val1 = lineSplit[0];
            outputs.val2 = QString(lineSplit[1]).toDouble();
            outputs.val3 = QString(lineSplit[2]).toDouble();
            outputs.val4 = QString(lineSplit[3]).toDouble();
            outputs.val5 = QString(lineSplit[4]).toDouble();
            outputs.val6 = QString(lineSplit[5]).toDouble();
            outputs.val7 = QString(lineSplit[6]).toDouble();
            outputs.val8 = QString(lineSplit[7]).toDouble();
            outputs.val9 = QString(lineSplit[8]).toDouble();
            outputs.val10 = QString(lineSplit[9]).toDouble();
            outputs.val11 = QString(lineSplit[10]).toDouble();
            outputs.val12 = QString(lineSplit[11]).toDouble();
            outputs.dxaHipTScore = QString(lineSplit[12]).toDouble();
            outputs.fracRisk1 = QString(lineSplit[13]).toDouble();
            outputs.fracRisk2 = QString(lineSplit[14]).toDouble();
            outputs.fracRisk3 = QString(lineSplit[15]).toDouble();
            outputs.fracRisk4 = QString(lineSplit[16]).toDouble();
        }
        return outputs;
    }
    return OutputsModel();
}
