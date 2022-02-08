#include "BloodPressureMeasurement.h"

#include <QDebug>
#include <QRandomGenerator>

/**
 * sample input
 *
 *
 */



void BloodPressureMeasurement::storeData(const int sbp, const int dbp, const int pulse, const bool isAverage)
{
    setCharacteristic("sbp", sbp);
    setCharacteristic("dbp", dbp);
    setCharacteristic("pulse", pulse);
    setCharacteristic("is average", isAverage);
}

bool BloodPressureMeasurement::isValid() const
{
    bool hasAllRequiredCharacteristics =
        hasCharacteristic("sbp") 
        && hasCharacteristic("dbp")
        && hasCharacteristic("pulse")
        && hasCharacteristic("is average");
    if (hasAllRequiredCharacteristics == false) {
            return false;
    }

    // TODO: There should be some low and high values which
    //       are not feasible to be blood pressures. These 
    //       values should be set as boundries for check
    bool sbpValid = getCharacteristic("sbp").toInt() > -1;
    bool dbpValid = getCharacteristic("dbp").toInt() > -1;
    bool pulseValid = getCharacteristic("pulse").toInt() > -1;
    return sbpValid && dbpValid && pulseValid;
}

QString BloodPressureMeasurement::toString() const
{
    int m_sbp = getCharacteristic("sbp").toInt();
    int m_dbp = getCharacteristic("dbp").toInt();
    int m_pulse = getCharacteristic("pulse").toInt();
    int m_isAverage = getCharacteristic("is average").toInt();
    return QString("SBP: %1 DBP: %2 Pulse: %3 (%4)").arg(m_sbp).arg(m_dbp).arg(m_pulse).arg(m_isAverage? "Average": "Reading");
}

bool BloodPressureMeasurement::hasValues(const int sbp, const int dbp, const int pulse, const bool isAverage)
{
    int m_sbp = getCharacteristic("sbp").toInt();
    int m_dbp = getCharacteristic("dbp").toInt();
    int m_pulse = getCharacteristic("pulse").toInt();
    int m_isAverage = getCharacteristic("is average").toInt();
    return m_sbp == sbp && m_dbp == dbp && m_pulse == pulse && isAverage == isAverage;
}

BloodPressureMeasurement BloodPressureMeasurement::simulate()
{
    BloodPressureMeasurement simulatedMeasurement;
    return simulatedMeasurement;
}

QDebug operator<<(QDebug dbg, const BloodPressureMeasurement& item)
{
    const QString measurementStr = item.toString();
    if (measurementStr.isEmpty())
        dbg.nospace() << "Blood Pressure Measurement()";
    else
        dbg.nospace() << "Blood Pressure Measurement(" << measurementStr << " ...)";
    return dbg.maybeSpace();
}