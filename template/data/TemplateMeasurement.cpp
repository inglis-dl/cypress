#include "TemplateMeasurement.h"

#include <QDebug>

bool TemplateMeasurement::isValid() const
{
    return true;
}

QString TemplateMeasurement::toString() const
{
    return QString();
}

TemplateMeasurement TemplateMeasurement::simulate()
{
    TemplateMeasurement simulatedMeasurement;
    return simulatedMeasurement;
}

QDebug operator<<(QDebug dbg, const TemplateMeasurement& item)
{
    const QString measurementStr = item.toString();
    if (measurementStr.isEmpty())
        dbg.nospace() << "Template Measurement()";
    else
        dbg.nospace() << "Template Measurement(" << measurementStr << " ...)";
    return dbg.maybeSpace();
}
