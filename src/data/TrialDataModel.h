#ifndef TRIALDATAMODEL_H
#define TRIALDATAMODEL_H

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
	//QList<double> flowValues;
	QString flowValues;
	double volumeInterval = -1;
	//QList<double> volumeValues;
	QString volumeValues;
};

#endif // TRIALDATAMODEL_H
