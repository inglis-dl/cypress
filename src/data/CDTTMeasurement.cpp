#include "CDTTMeasurement.h"

#include <QDebug>

bool CDTTMeasurement::isValid() const
{
    bool ok =
            hasCharacteristic("name") &&
            hasCharacteristic("value");
    if(ok)
    {
        QStringList list = {"speech_reception_threshold","standard_deviation","reversal_count","trial_count"};
        QString name = getCharacteristic("name").toString();
        ok = (list.contains(name) || name.startsWith("stimulus") || name.startsWith("response"));
    }
    return ok;
}

QString CDTTMeasurement::toString() const
{
  QString s;
  if(isValid())
  {
    s = QString("%1: %2").arg(
          getCharacteristic("name").toString(),
          getCharacteristic("value").toString());
  }
  return s;
}

QDebug operator<<(QDebug dbg, const CDTTMeasurement &item)
{
    const QString measurementStr = item.toString();
    if (measurementStr.isEmpty())
        dbg.nospace() << "CDTTT Measurement()";
    else
        dbg.nospace() << "CDTTT Measurement(" << measurementStr << " ...)";
    return dbg.maybeSpace();
}
