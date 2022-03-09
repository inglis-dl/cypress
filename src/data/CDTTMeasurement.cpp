#include "CDTTMeasurement.h"

#include <QDebug>

bool CDTTMeasurement::isValid() const
{
    bool ok =
      hasAttribute("trial");
    return ok;
}

QString CDTTMeasurement::toString() const
{
  QString s;
  if(isValid())
  {
    s = getAttribute("trial").toString();
  }
  /*
  if(isValid())
  {
    QString name = getAttribute("name").toString();
    QVariant value = getAttribute("value");
    QString representation;
    if(QVariant::List == value.type())
    {
        value.convert(QVariant::StringList);
        representation = value.toStringList().join(",");
    }
    else
        representation = value.toString();

    s = QString("%1: %2").arg(name, representation);
  }
  */
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
