#include "TonometerMeasurement.h"
#include <QDebug>

bool TonometerMeasurement::isValid() const
{
    bool ok =
      hasCharacteristic("name") &&
      hasCharacteristic("side");
    return ok;
}

QString TonometerMeasurement::toString() const
{
    QStringList skip = {"applanation,pressure,indexes"};
    QString s;
    if(isValid())
    {
      if(skip.contains(getCharacteristic("name").toString()))
      {
        s = QString("%1 %2: %3...").arg(
          getCharacteristic("side").toString(),
          getCharacteristic("name").toString(),
          getCharacteristic("value").toString().left(10));
      }
      else
      {
        if(getCharacteristic("units").isNull())
        {
          s = QString("%1 %2: %3").arg(
            getCharacteristic("side").toString(),
            getCharacteristic("name").toString(),
            getCharacteristic("value").toString());
        }
        else
        {
          s = QString("%1 %2: %3(%4)").arg(
            getCharacteristic("side").toString(),
            getCharacteristic("name").toString(),
            getCharacteristic("value").toString(),
            getCharacteristic("units").toString());
        }
      }
    }
    return s;
}

QDebug operator<<(QDebug dbg, const TonometerMeasurement &item)
{
    const QString s = item.toString();
    if (s.isEmpty())
        dbg.nospace() << "Tonometer Measurement()";
    else
        dbg.nospace() << "Tonometer Measurement(" << s << " ...)";
    return dbg.maybeSpace();
}
