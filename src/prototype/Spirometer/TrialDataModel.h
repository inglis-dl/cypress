#pragma once

#include <QString>
#include <QDate>
#include <QList>

#include "ResultParametersModel.h"

class TrialDataModel
{
public:
	QDateTime date;
	int number;
	int rank;
	int rankOriginal;
	QString accepted;
	QString acceptedOriginal;
	QString manualAmbientOverride;
	ResultParametersModel resultParameters;
	double flowInterval = -1;
	QList<double> flowValues;
	double volumeInterval = -1;
	QList<double> volumeValues;
};

