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
      hasCharacteristic("body type") &&
      hasCharacteristic("gender") &&
      hasCharacteristic("units") &&
      hasCharacteristic("impedence") &&
      hasCharacteristic("percet fat") &&
      hasCharacteristic("fat mass") &&
      hasCharacteristic("ffm") &&
      hasCharacteristic("tbw") &&
      hasCharacteristic("age") &&
      hasCharacteristic("bmi") &&
      hasCharacteristic("bmr");
    return ok;
}

QString BodyCompositionMeasurement::toString() const
{
  QString s;
  if(isValid())
  {
    s = getCharacteristic("body type").toString() %
      QString(" ") %
      getCharacteristic("gender").toString() %
      QString(" ") %
      getCharacteristic("units").toString() %
      QString(" ") %
      getCharacteristic("impedence").toString() %
      QString(" ") %
      getCharacteristic("percent fat").toString() %
      QString(" ") %
      getCharacteristic("fat mass").toString() %
      QString(" ") %
      getCharacteristic("ffm").toString() %
      QString(" ") %
      getCharacteristic("tbw").toString() %
      QString(" ") %
      getCharacteristic("age").toString() %
      QString(" ") %
      getCharacteristic("bmi").toString() %
      QString(" ") %
      getCharacteristic("bmr").toString();
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
