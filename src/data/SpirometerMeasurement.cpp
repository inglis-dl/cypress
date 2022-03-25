#include "SpirometerMeasurement.h"

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

// Create a list of all the elements that should appear first when creating a string
QStringList firstKeysToAddToString = { trialNumber, outputTrialDate, outputTrialRank, outputTrialRankOriginal, accepted,
    acceptedOriginal, flowInterval, flowValues, volumeInterval, volumeValues, manualAmbientOverride };

// These are the result parameters that should have there normal
// and predicted values stored as measurements
const QList<QString> storeNormalPredictedAsMeasureList = 
    {"fef2575", "fev1_fev6","fev6","mmef", "pef","pef_l_min"};

// These are the result parameters that should have there normal
// and predicted values stored as meta data
const QList<QString> storeNormalPredictedAsMetaList = { "fev1", "fev1_fvc","fvc" };

/**
 * sample input
 *
 *
 */

void SpirometerMeasurement::fromTrialData(const TrialDataModel& trialData)
{
    setAttribute(outputTrialDate, trialData.date);
    setAttribute(outputTrialRank, trialData.rank);
    setAttribute(outputTrialRankOriginal, trialData.rankOriginal);
    setAttribute(accepted, trialData.accepted);
    setAttribute(acceptedOriginal, trialData.acceptedOriginal);
    setAttribute(manualAmbientOverride, trialData.manualAmbientOverride);
    setAttribute(flowInterval, trialData.flowInterval);
    setAttribute(flowValues, trialData.flowValues);
    setAttribute(trialNumber, trialData.number);
    setAttribute(volumeInterval, trialData.volumeInterval);
    setAttribute(volumeValues, trialData.volumeValues);

    foreach(const auto key, trialData.resultParameters.results.keys())
    {
        qDebug() << key;
        ResultParameterModel result = trialData.resultParameters.results[key];
        setAttribute(QString("%1_data_value").arg(key.toLower()), result.dataValue, result.unit);

        // If the key is in the list of elements that need to store normal 
        // and predicted values as measurements, then store those vales as well
        if(storeNormalPredictedAsMeasureList.contains(key.toLower()))
        {
            setAttribute(QString("%1_ll_normal_value").arg(key.toLower()), result.llNormalValue, result.unit);
            setAttribute(QString("%1_predicted_value").arg(key.toLower()), result.predictedValue, result.unit);
        }
        // If the key is in the list of elements that need to store normal 
        // and predicted values in meta, then store those vales in a temp meta list
        else if(storeNormalPredictedAsMetaList.contains(key.toLower()))
        {
            metaResults.results.insert(key, result);
        }
    }
}

bool SpirometerMeasurement::isValid() const
{
    return true;
}

QString SpirometerMeasurement::toString() const
{
    QString output = "";

    // add the first keys to the list
    foreach(const auto key, firstKeysToAddToString)
    {
        appendMeasurementAttribute(&output, key);
    }

    // add the rest of the elements to the list
    QStringList keys = getAttributes().keys();
    foreach(const auto key, keys)
    {
        // Skip any of the previously entered values
        if(firstKeysToAddToString.contains(key))
        {
            continue;
        }
        appendMeasurementAttribute(&output, key);
    }

    // an extra comma will likely be at the start unless there is nothing in the string
    if(output.startsWith(","))
    {
        output.remove(0, 1);
    }

    return output;
}

QStringList SpirometerMeasurement::getHeaderValues() const
{
    QStringList output;

    // add the first keys to the list
    foreach(const auto key, firstKeysToAddToString)
    {
        output.append(key);
    }

    // add the rest of the elements to the list
    QStringList keys = getAttributes().keys();
    foreach(const auto key, keys)
    {
        // Skip any of the previously entered values
        if(firstKeysToAddToString.contains(key))
        {
            continue;
        }
        output.append(key);
    }

    return output;
}

SpirometerMeasurement SpirometerMeasurement::simulate()
{
    SpirometerMeasurement simulatedMeasurement;
    return simulatedMeasurement;
}

void SpirometerMeasurement::appendMeasurementAttribute(QString* measurementStr, const QString& key) const
{
    QString value = getAttribute(key).value().toString();
    if(QStringList{ flowValues, volumeValues }.contains(key))
    {
        value = QString("%1 ...").arg(value.split(" ")[0]);
    }
    *measurementStr = QString("%1, %2").arg(*measurementStr,value);

    // Add the units if it is stored
    if(getAttribute(key).hasUnits())
    {
        QString units = getAttribute(key).units();
        *measurementStr = QString("%1 (%2)").arg(*measurementStr,units);
    }
}

QDebug operator<<(QDebug dbg, const SpirometerMeasurement& item)
{
    const QString measurementStr = item.toString();
    if(measurementStr.isEmpty())
        dbg.nospace() << "Template Measurement()";
    else
        dbg.nospace() << "Template Measurement(" << measurementStr << " ...)";
    return dbg.maybeSpace();
}
