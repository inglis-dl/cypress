#include "TrueFlowSpirometerMeasurement.h"

#include <QDebug>
#include <QRandomGenerator>

/**
 * sample input
 *
 *
 */

void TrueFlowSpirometerMeasurement::fromString(const QString& commaSeperatedLine)
{
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