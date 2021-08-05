#pragma once
#include "InputsModel.h"
#include "ExitCodesEnum.h"
#include "Strings.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>

class CognitiveIO
{
public:
	static void ReadJsonInputs(QString path, InputsModel* inputs);
	static int CreateInputsModelFromCmdArgs(int argc, char* argv[], InputsModel* commandLineInputs);
private:
	static int AddArgToInputs(int argNum, InputsModel* inputs, QString* argument);
	static int DetermineExitCode(int argNum, bool test, bool fullTest);
	static bool ValidateUserIDCmdArg(QString userID);
	static bool ValidateSiteNameCmdArg(QString siteName);
	static bool ValidateInterviewerIDCmdArg(QString interviewerID);
	static bool ValidateLanguageCmdArg(QString language);
};

