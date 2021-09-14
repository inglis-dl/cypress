#include "Measurement.h"

#include <QDebug>

Measurement::Measurement(const Measurement &other)
{
    m_units = other.m_units;
    m_value = other.m_value;
    m_name = other.m_name;
    m_mode = other.m_mode;
    m_timestamp = other.m_timestamp;
}

Measurement& Measurement::operator=(const Measurement &other)
{
    m_units = other.m_units;
    m_value = other.m_value;
    m_name = other.m_name;
    m_mode = other.m_mode;
    m_timestamp = other.m_timestamp;
    return *this;
}

void Measurement::fromArray(const QByteArray &arr)
{
    if(!arr.isEmpty())
    {
      QByteArray bytes(arr.simplified());

      QList<QByteArray> parts = bytes.split(' ');
      if(3<=parts.size())
      {
        m_value = parts[0];
        m_units = QString(parts[1]);
        m_mode = QString(parts[2]);
        m_timestamp = QDateTime::currentDateTime();
      }
    }
}

bool Measurement::isValid() const
{
    bool ok;
    float fval = m_value.toFloat(&ok);
    return ok
      && !m_units.isEmpty()
      && !m_mode.isEmpty()
      && m_timestamp.isValid()
      && !m_name.isEmpty()
      && 0.0f <= fval;
}

bool Measurement::isZero() const
{
    return isValid() && 0.0f == m_value.toFloat();
}

void Measurement::reset()
{
    m_value.clear();
    m_timestamp = QDateTime();
}

QString Measurement::toString() const
{
    QStringList list;
    QString d = m_timestamp.date().toString("yyyy-MM-dd");
    QString t = m_timestamp.time().toString("hh:mm:ss");
    list << m_value.toString() << m_units << m_mode << d << t;
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

