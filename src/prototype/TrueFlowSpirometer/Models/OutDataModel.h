#pragma once

#include <QString>
#include <QList>
#include <QDate>
#include <QDateTime>

#include "TrialDataModel.h"

class OutDataModel
{
public:
	double height = -1;
	double weight = -1;
	QString ethnicity = "";
	QString asthma = "";
	QString smoker = "";
	QString copd = "";
	QString gender = "";
	QDate birthDate;
	
	QDateTime testDate;
	ResultParametersModel bestValues;
	QString softwareVersion = "";
	QString qualityGrade = "";
	QString qualityGradeOriginal = "";
	QList<TrialDataModel> trials;
	QString pdfPath = "";
};

