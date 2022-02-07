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
    m_sbp = sbp;
    m_dbp = dbp;
    m_pulse = pulse;
    m_isAverage = isAverage;
}

bool BloodPressureMeasurement::isValid() const
{
    // TODO: There should be some low and high values which
    //       are not feasible to be blood pressures. These 
    //       values should be set as boundries for check
    bool sbpValid = m_sbp > -1;
    bool dbpValid = m_dbp > -1;
    bool pulseValid = m_pulse > -1;
    return sbpValid && dbpValid && pulseValid;
}

QString BloodPressureMeasurement::toString() const
{
    return QString("SBP: %1 DBP: %2 Pulse: %3 (%4)").arg(m_sbp).arg(m_dbp).arg(m_pulse).arg(m_isAverage? "Average": "Reading");
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