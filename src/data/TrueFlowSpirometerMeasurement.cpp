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
    m_characteristicValues.insert(outputTrialDate, trialData.date);
    m_characteristicValues.insert(outputTrialRank, trialData.rank);
    m_characteristicValues.insert(outputTrialRankOriginal, trialData.rankOriginal);
    m_characteristicValues.insert(accepted, trialData.accepted);
    m_characteristicValues.insert(acceptedOriginal, trialData.acceptedOriginal);
    m_characteristicValues.insert(manualAmbientOverride, trialData.manualAmbientOverride);
    m_characteristicValues.insert(flowInterval, trialData.flowInterval);
    QVariant flowVals = QVariant::fromValue<QList<double>>(trialData.flowValues);
    m_characteristicValues.insert(flowValues, flowVals);
    m_characteristicValues.insert(trialNumber, trialData.number);
    m_characteristicValues.insert(volumeInterval, trialData.volumeInterval);
    QVariant volumeVals = QVariant::fromValue<QList<double>>(trialData.volumeValues);
    m_characteristicValues.insert(volumeValues, volumeVals);

    for (QString key : trialData.resultParameters.results.keys())
    {
        qDebug() << key << endl;
        ResultParameterModel result = trialData.resultParameters.results[key];
        m_characteristicValues.insert(QString("%1_data_value").arg(key.toLower()), result._dataValue);
        m_characteristicValues.insert(QString("%1_ll_normal_value").arg(key.toLower()), result._llNormalValue);
        m_characteristicValues.insert(QString("%1_predicted_value").arg(key.toLower()), result._predictedValue);
        m_characteristicValues.insert(QString("%1_unit").arg(key.toLower()), result._unit);
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