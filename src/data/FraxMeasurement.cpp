#include "FraxMeasurement.h"

#include <QDebug>

bool FraxMeasurement::isValid() const
{
    bool ok =
      hasCharacteristic("units") &&
      hasCharacteristic("type") &&
      hasCharacteristic("probability");
    return ok;
}

QString FraxMeasurement::toString() const
{
    QString s;
    if(isValid())
    {
        s = QString("%1: %2(%3)")
            .arg(
                getCharacteristic("type").toString(),
                QString::number(getCharacteristic("probability").toDouble()),
                getCharacteristic("units").toString()
            );
    }
    return s;
}

QDebug operator<<(QDebug dbg, const FraxMeasurement &item)
{
    const QString s = item.toString();
    if (s.isEmpty())
        dbg.nospace() << "Frax Measurement()";
    else
        dbg.nospace() << "Frax Measurement(" << s << " ...)";
    return dbg.maybeSpace();
}
