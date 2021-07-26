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
	static InputsModel ReadInputs(QString path);
private:
	static InputsModel CreateInputsModel(QJsonObject* json);
};

