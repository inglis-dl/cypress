#include "BloodPressureMeasurement.h"

#include <QDebug>
#include <QDateTime>

BloodPressureMeasurement::BloodPressureMeasurement(const int &sbp, const int& dbp, const int& pulse, const QDateTime &start, const QDateTime& end, const int& readingNum)
{
    setCharacteristic("reading_number", readingNum);
    setCharacteristic("start_time", start);
    setCharacteristic("end_time", end);
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
        && hasCharacteristic("start_time")
        && hasCharacteristic("end_time")
        && hasCharacteristic("reading_number");
    if(!hasAllRequiredCharacteristics)
    {
      return false;
    }

    // TODO: There should be some low and high values which
    //       are not feasible to be blood pressures. These 
    //       values should be set as boundries for check
    //       normal systolic pressure: 90 - 120 mmHg
    //       normal diastolic pressure: 60 - 80 mmHg
    //       normal pulse rate: 60 - 100 bpm
    //
    bool sbpValid = 0 < getCharacteristic("systolic").toInt();
    bool dbpValid = 0 < getCharacteristic("diastolic").toInt();
    bool pulseValid = 0 < getCharacteristic("pulse").toInt();
    return (sbpValid && dbpValid && pulseValid);
}

QString BloodPressureMeasurement::toString() const
{
    return QString("%4. SBP: %1 DBP: %2 Pulse: %3 (%5 -> %6)")
        .arg(getSbp())
        .arg(getDbp())
        .arg(getPulse())
        .arg(getCharacteristic("reading_number").toInt())
        .arg(getCharacteristic("start_time").toDateTime().toString("yyyy-MM-dd  HH:mm:ss"))
        .arg(getCharacteristic("end_time").toDateTime().toString("HH:mm:ss"));
}

// TODO: implement simulated measurement
//
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
