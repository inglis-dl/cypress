#include "Measurement.h"

#include <QDateTime>
#include <QDebug>

Measurement::Measurement(const Measurement &other)
{
    m_characteristicValues = other.m_characteristicValues;
}

Measurement& Measurement::operator=(const Measurement &other)
{
    m_characteristicValues = other.m_characteristicValues;
    return *this;
}

void Measurement::fromArray(const QByteArray &arr)
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

bool Measurement::isValid() const
{
    bool ok;
    float fval = m_characteristicValues["weight"].toFloat(&ok);
    return ok
      && !m_characteristicValues["units"].toString().isEmpty()
      && !m_characteristicValues["mode"].toString().isEmpty()
      && m_characteristicValues["timestamp"].toDateTime().isValid()
      && 0.0f <= fval;
}

bool Measurement::isZero() const
{
    return isValid() && 0.0f == m_characteristicValues["weight"].toFloat();
}

void Measurement::reset()
{
    m_characteristicValues.clear();
}

QString Measurement::toString() const
{
    QDateTime dt = m_characteristicValues["timestamp"].toDateTime();
    QString w = m_characteristicValues["weight"].toString();
    QString u = m_characteristicValues["units"].toString();
    QString m = m_characteristicValues["mode"].toString();
    QStringList list;
    QString d = dt.date().toString("yyyy-MM-dd");
    QString t = dt.time().toString("hh:mm:ss");
    list << w << u << m << d << t;
    return list.join(" ");
}

QDebug operator<<(QDebug dbg, const Measurement &measurement)
{
    const QString output = measurement.toString();
    if (output.isEmpty())
        dbg.nospace() << "Measurement()";
    else
        dbg.nospace() << "Measurement(" << output << " ...)";
    return dbg.maybeSpace();
}

