#ifndef RESULTSPARAMETERMODEL_H
#define RESULTSPARAMETERMODEL_H

#include "ResultParameterModel.h"

#include <QMap>

class ResultParametersModel
{
public:
	QMap<QString, ResultParameterModel> results;
};

#endif // RESULTSPARAMETERMODEL_H
