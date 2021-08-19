#pragma once
#include <QString>
#include <QStringList>

#include "ModesEnum.h"

class InputsModel
{
public:
	int mode = Modes::unselected;
	QString jsonInputsPath = "";
	QString jsonOutputsPath = "";

	QString CCBFolderPath = "C:/Program Files (x86)/Cardiff_University/CCB";

	QString userID = "";
	QString dcsSiteName = "";
	QString interviewerID = "";
	QString language = "";
};