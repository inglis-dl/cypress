#pragma once
#include "InputsModel.h"
#include "ExitCodesEnum.h"
#include "ModesEnum.h"
#include "Strings.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QCommandLineParser>

class CognitiveIO
{
public:
	static int ParseCommandLineArgs(QApplication* app, InputsModel* commandLineInputs);
private:
	static void ParseCommandLineInputs(QApplication* app, InputsModel* inputs);
	static int AddJsonInputs(InputsModel* inputs);
	static int DetermineExitCode(InputsModel* inputs);
};

