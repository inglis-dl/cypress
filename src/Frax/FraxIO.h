#pragma once

#include "InputsModel.h"
#include "OutputsModel.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>

class FraxIO: public QObject
{
	Q_OBJECT
public:
	static InputsModel ReadInputs(QString readPath);
	static bool CreateInputTxt(InputsModel* inputs);
	static OutputsModel ReadOutputs(QString readPath);
private:
	static InputsModel CreateInputsModel(QJsonObject* json);
};

