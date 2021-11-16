#include "BodyCompositionMeasurement.h"

#include <QDateTime>
#include <QDebug>
#include <QRegExp>
#include <QStringBuilder>

bool BodyCompositionMeasurement::isValid() const
{
    // TODO: measurement break out to meta data
    // and individual components
  bool ok =
    (
      hasCharacteristic("weight") ||
      hasCharacteristic("impedence") ||
      hasCharacteristic("percent fat") ||
      hasCharacteristic("fat mass") ||
      hasCharacteristic("fat free mass") ||
      hasCharacteristic("total body water") ||
      hasCharacteristic("body mass index") ||
      hasCharacteristic("bmr")
    ) &&
    hasCharacteristic("units");

    return ok;
}

QString BodyCompositionMeasurement::toString() const
{
  QString s;
  if(isValid())
  {
      if(hasCharacteristic("weight"))
        s = "weight: " % getCharacteristic("weight").toString() % QString(" ");
      else if(hasCharacteristic("impedence"))
        s = "impedence: " %getCharacteristic("impedence").toString() % QString(" ");
      else if(hasCharacteristic("percent fat"))
        s = "percent fat: " %getCharacteristic("percent fat").toString() % QString(" ");
      else if(hasCharacteristic("fat mass"))
        s = "fat mass: " %getCharacteristic("fat mass").toString() % QString(" ");
      else if(hasCharacteristic("fat free mass"))
        s = "fat free mass: " %getCharacteristic("fat free mass").toString() % QString(" ");
      else if(hasCharacteristic("total body water"))
        s = "total body water: " %getCharacteristic("total body water").toString() % QString(" ");
      else if(hasCharacteristic("body mass index"))
        s = "body mass index: " %getCharacteristic("body mass index").toString() % QString(" ");
      else if(hasCharacteristic("bmr"))
        s = "bmr: " %getCharacteristic("bmr").toString() % QString(" ");
     s = s % QString("(") % getCharacteristic("units").toString() % QString(")");
  }
  return s;
}

QDebug operator<<(QDebug dbg, const BodyCompositionMeasurement &item)
{
    const QString s = item.toString();
    if (s.isEmpty())
        dbg.nospace() << "Body Composition Measurement()";
    else
        dbg.nospace() << "Body Composition Measurement(" << s << " ...)";
    return dbg.maybeSpace();
}
