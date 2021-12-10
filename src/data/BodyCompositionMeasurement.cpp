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
      hasCharacteristic("impedance") ||
      hasCharacteristic("percent fat") ||
      hasCharacteristic("fat mass") ||
      hasCharacteristic("fat free mass") ||
      hasCharacteristic("total body water") ||
      hasCharacteristic("body mass index") ||
      hasCharacteristic("basal metabolic rate")
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
      else if(hasCharacteristic("impedance"))
        s = "impedance: " %getCharacteristic("impedance").toString() % QString(" ");
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
      else if(hasCharacteristic("basal metabolic rate"))
        s = "basal metabolic rate: " %getCharacteristic("basal metabolic rate").toString() % QString(" ");

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
