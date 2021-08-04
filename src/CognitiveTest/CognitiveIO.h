#pragma once
#include "InputsModel.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>

class CognitiveIO
{
public:
	static void ReadJsonInputs(QString path, InputsModel* inputs);
	static InputsModel ReadCommandLineInputs(int argc, char* argv[]);
private:
	static void AddArgToInputs(int argNum, int* unexpectedArgs, InputsModel* inputs, QString* argument);
};

