#include "BloodPressureMeasurement.h"

#include <QDebug>
#include <QRandomGenerator>

/**
 * sample input
 *
 *
 */
void BloodPressureMeasurement::storeData(const int &sbp, const int& dbp, const int& pulse, const QDateTime &start, const QDateTime& end, const int& readingNum)
{
    setCharacteristic("reading number", readingNum);
    setCharacteristic("start time", start);
    setCharacteristic("end time", end);
    setCharacteristic("systolic", sbp);
    setCharacteristic("diastolic", dbp);
    setCharacteristic("pulse", pulse);
}

bool BloodPressureMeasurement::isValid() const
{
    bool hasAllRequiredCharacteristics =
        hasCharacteristic("systolic") 
        && hasCharacteristic("diastolic")
        && hasCharacteristic("pulse")
        && hasCharacteristic("start time")
        && hasCharacteristic("end time")
        && hasCharacteristic("reading number");
    if (hasAllRequiredCharacteristics == false) {
            return false;
    }

    // TODO: There should be some low and high values which
    //       are not feasible to be blood pressures. These 
    //       values should be set as boundries for check
    bool sbpValid = getCharacteristic("systolic").toInt() > -1;
    bool dbpValid = getCharacteristic("diastolic").toInt() > -1;
    bool pulseValid = getCharacteristic("pulse").toInt() > -1;
    return sbpValid && dbpValid && pulseValid;
}

QString BloodPressureMeasurement::toString() const
{
    return QString("%4. SBP: %1 DBP: %2 Pulse: %3 (%5 -> %6)")
        .arg(getSbp())
        .arg(getDbp())
        .arg(getPulse())
        .arg(getCharacteristic("reading number").toInt())
        .arg(getCharacteristic("start time").toDateTime().toString("yyyy-MM-dd  HH:mm:ss"))
        .arg(getCharacteristic("end time").toDateTime().toString("HH:mm:ss"));
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