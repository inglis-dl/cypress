#include "BloodPressureMeasurement.h"

#include <QDebug>
#include <QRandomGenerator>

/**
 * sample input
 *
 *
 */

void BloodPressureMeasurement::fromString(const QString& commaSeperatedLine)
{
}

bool BloodPressureMeasurement::isValid() const
{
    return true;
}

QString BloodPressureMeasurement::toString() const
{
    return "";
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