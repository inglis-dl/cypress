#include "FraxMeasurement.h"

#include <QDebug>

bool FraxMeasurement::isValid() const
{
    bool ok =
      hasAttribute("type") &&
      hasAttribute("probability");
    return ok;
}

QString FraxMeasurement::toString() const
{
    qDebug() << "making string";
    QString s;
    if(isValid())
    {
      QString type = getAttribute("type").toString().replace("_"," ");
      QString p = QString::number(getAttributeValue("probability").toDouble(),'f',1);
      s = QString("%1 risk: %2 (%3)").arg(type,p,getAttribute("probability").units());
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
