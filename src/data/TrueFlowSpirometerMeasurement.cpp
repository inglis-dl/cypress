#include "TrueFlowSpirometerMeasurement.h"

#include <QDebug>
#include <QRandomGenerator>

const QString outputTrialDate = "output_trial_date";
const QString outputTrialRank = "output_trial_rank";
const QString outputTrialRankOriginal = "output_trial_rank_original";
const QString accepted = "accepted";
const QString acceptedOriginal = "accepted_original";
const QString manualAmbientOverride = "manual_ambient_override";
const QString flowInterval = "flow_interval";
const QString flowValues = "flow_values";
const QString trialNumber = "trial_number";
const QString volumeInterval = "volume_interval";
const QString volumeValues = "volume_values";

/**
 * sample input
 *
 *
 */

void TrueFlowSpirometerMeasurement::fromTrialData(const TrialDataModel& trialData)
{
    setAttribute(outputTrialDate, trialData.date);
    setAttribute(outputTrialRank, trialData.rank);
    setAttribute(outputTrialRankOriginal, trialData.rankOriginal);
    setAttribute(accepted, trialData.accepted);
    setAttribute(acceptedOriginal, trialData.acceptedOriginal);
    setAttribute(manualAmbientOverride, trialData.manualAmbientOverride);
    setAttribute(flowInterval, trialData.flowInterval);
    QVariant flowVals = QVariant::fromValue<QList<double>>(trialData.flowValues);
    setAttribute(flowValues, flowVals);
    setAttribute(trialNumber, trialData.number);
    setAttribute(volumeInterval, trialData.volumeInterval);
    QVariant volumeVals = QVariant::fromValue<QList<double>>(trialData.volumeValues);
    setAttribute(volumeValues, volumeVals);

    for (QString key : trialData.resultParameters.results.keys())
    {
        qDebug() << key << endl;
        ResultParameterModel result = trialData.resultParameters.results[key];
        setAttribute(QString("%1_data_value").arg(key.toLower()), result._dataValue, result._unit);
        setAttribute(QString("%1_ll_normal_value").arg(key.toLower()), result._llNormalValue, result._unit);
        setAttribute(QString("%1_predicted_value").arg(key.toLower()), result._predictedValue, result._unit);
    }
}

bool TrueFlowSpirometerMeasurement::isValid() const
{
    return true;
}

QString TrueFlowSpirometerMeasurement::toString() const
{
    return "";
}

TrueFlowSpirometerMeasurement TrueFlowSpirometerMeasurement::simulate()
{
    TrueFlowSpirometerMeasurement simulatedMeasurement;
    return simulatedMeasurement;
}

QDebug operator<<(QDebug dbg, const TrueFlowSpirometerMeasurement& item)
{
    const QString measurementStr = item.toString();
    if (measurementStr.isEmpty())
        dbg.nospace() << "Template Measurement()";
    else
        dbg.nospace() << "Template Measurement(" << measurementStr << " ...)";
    return dbg.maybeSpace();
}