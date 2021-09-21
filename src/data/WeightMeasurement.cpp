#include "WeightMeasurement.h"

#include <QDateTime>

void WeightMeasurement::fromArray(const QByteArray &arr)
{
    if(!arr.isEmpty())
    {
      QByteArray bytes(arr.simplified());
      QList<QByteArray> parts = bytes.split(' ');
      if(3 <= parts.size())
      {
        m_characteristicValues["weight"] = QString::number(parts[0].toFloat(),'f',1);
        m_characteristicValues["units"] = QString(parts[1]);
        m_characteristicValues["mode"] = QString(parts[2]);
        m_characteristicValues["timestamp"] = QDateTime::currentDateTime();
      }
    }
}

bool WeightMeasurement::isValid() const
{
    bool ok;
    float fval = m_characteristicValues["weight"].toFloat(&ok);
    return ok
      && !m_characteristicValues["units"].toString().isEmpty()
      && !m_characteristicValues["mode"].toString().isEmpty()
      && m_characteristicValues["timestamp"].toDateTime().isValid()
      && 0.0f <= fval;
}

bool WeightMeasurement::isZero() const
{
    return isValid() && 0.0f == m_characteristicValues["weight"].toFloat();
}

QString WeightMeasurement::toString() const
{
    QString w = m_characteristicValues["weight"].toString();
    QString u = m_characteristicValues["units"].toString();
    QString m = m_characteristicValues["mode"].toString();
    QDateTime dt = m_characteristicValues["timestamp"].toDateTime();
    QString d = dt.date().toString("yyyy-MM-dd");
    QString t = dt.time().toString("hh:mm:ss");

    QStringList list;
    list << w << u << m << d << t;
    return list.join(" ");
}
