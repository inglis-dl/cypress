#include "GripStrengthMeasurement.h"

#include <QDebug>

bool GripStrengthMeasurement::isValid() const
{
    return true;
}

QString GripStrengthMeasurement::toString() const
{
    return QString();
}

GripStrengthMeasurement GripStrengthMeasurement::simulate()
{
    GripStrengthMeasurement simulatedMeasurement;
    return simulatedMeasurement;
}

QDebug operator<<(QDebug dbg, const GripStrengthMeasurement& item)
{
    const QString measurementStr = item.toString();
    if (measurementStr.isEmpty())
        dbg.nospace() << "GripStrength Measurement()";
    else
        dbg.nospace() << "GripStrength Measurement(" << measurementStr << " ...)";
    return dbg.maybeSpace();
}
