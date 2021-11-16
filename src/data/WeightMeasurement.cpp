#include "WeightMeasurement.h"

#include <QDateTime>
#include <QDebug>

void WeightMeasurement::fromArray(const QByteArray &arr)
{
    if(!arr.isEmpty())
    {
      QByteArray bytes(arr.simplified());
      QList<QByteArray> parts = bytes.split(' ');
      if(3 <= parts.size())
      {
        setCharacteristic("weight", QString::number(parts[0].toFloat(),'f',1));
        setCharacteristic("units", QString(parts[1]));
        setCharacteristic("mode", QString(parts[2]));
        setCharacteristic("timestamp", QDateTime::currentDateTime());
      }
    }
}

bool WeightMeasurement::isValid() const
{
    bool ok = false;
    if(hasCharacteristic("weight"))
    {
      float fval = getCharacteristic("weight").toFloat(&ok);
      ok = ok
        && hasCharacteristic("units")
        && hasCharacteristic("mode")
        && hasCharacteristic("timestamp")
        && 0.0f <= fval;
    }
    return ok;
}

bool WeightMeasurement::isZero() const
{
    return isValid() && 0.0f == getCharacteristic("weight").toFloat();
}

QString WeightMeasurement::toString() const
{
  QString s;
  if(isValid())
  {
    QString w = getCharacteristic("weight").toString();
    QString u = getCharacteristic("units").toString();
    QString m = getCharacteristic("mode").toString();
    QDateTime dt = getCharacteristic("timestamp").toDateTime();
    QString d = dt.date().toString("yyyy-MM-dd");
    QString t = dt.time().toString("hh:mm:ss");

    QStringList l;
    l << w << u << m << d << t;
    s = l.join(" ");
  }
  return s;
}

QDebug operator<<(QDebug dbg, const WeightMeasurement &item)
{
    const QString s = item.toString();
    if (s.isEmpty())
        dbg.nospace() << "Weight Measurement()";
    else
        dbg.nospace() << "Weight Measurement(" << s << " ...)";
    return dbg.maybeSpace();
}
