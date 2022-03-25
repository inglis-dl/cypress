#ifndef RESULTPARAMETERMODEL_H
#define RESULTPARAMETERMODEL_H

#include <QString>

class ResultParameterModel
{
public:
	double dataValue;
	QString unit;
	double predictedValue;
	double llNormalValue;
};

#endif // RESULTPARAMETERMODEL_H
