#include "Measurement.h"
#include "../auxiliary/Constants.h"

#include <QDebug>
#include <QJsonObject>

bool Measurement::isValid() const
{
    if(isEmpty())
    {
      return false;
    }
    bool ok = true;
    foreach(const auto x, m_attributes.toStdMap())
    {
      if(x.second.isNull())
      {
        ok = false;
        break;
      }
    }
    return ok;
}

void Measurement::reset()
{
    m_attributes.clear();
}

QStringList Measurement::toStringList(const bool& no_keys) const
{
    QStringList list;
    foreach(const auto x, m_attributes.toStdMap())
    {
      QString key = x.first; // the key
      Measurement::Value value = x.second; // the value
      list << (no_keys ? value.toString() : QString("%1: %2").arg(key,value.toString()));
    }
    return list;
}

QString Measurement::toString() const
{
    return toStringList().join(" ");
}

QJsonObject Measurement::toJsonObject() const
{
    QJsonObject json;
    foreach(const auto x, m_attributes.toStdMap())
    {
      // convert to space delimited phrases to snake_case
      //
      QString key = QString(x.first).toLower().replace(QRegExp("[\\s]+"),"_");
      Value v = x.second;
      QJsonValue jval = QJsonValue::fromVariant(v.value());
      if(v.hasUnits())
      {
          QJsonObject obj;
          obj.insert("value",jval);
          obj.insert("units",v.units());
          json.insert(key,obj);
      }
      else
        json.insert(key,jval);
    }
    return json;
}

QDebug operator<<(QDebug dbg, const Measurement &item)
{
    const QString s = item.toString();
    if(s.isEmpty())
      dbg.nospace() << "Measurement()";
    else
      dbg.nospace() << "Measurement(" << s << " ...)";
    return dbg.maybeSpace();
}

Measurement::Value::Value(const QVariant& value, const QString& units, const quint16& precision)
{
    m_value = value;
    if(!units.isEmpty() && 0 < units.length())
      m_units = units;
    m_precision = precision;
}

Measurement::Value::Value(const QVariant& value, const quint16& precision)
{
    m_value = value;
    m_precision = precision;
}

Measurement::Value::Value(const Measurement::Value& other)
{
    m_value = other.m_value;
    m_units = other.m_units;
    m_precision = other.m_precision;
}

Measurement::Value& Measurement::Value::operator=(const Value& other)
{
    if(*this != other)
    {
      m_value = other.m_value;
      m_units = other.m_units;
      m_precision = other.m_precision;
    }
    return *this;
}

bool Measurement::Value::hasUnits() const
{
    return (!m_units.isEmpty() && 0 < m_units.length());
}

bool Measurement::Value::isNull() const
{
   return m_value.isNull();
}

QString Measurement::Value::toString() const
{
    QString str;
    if(m_value.type() == QVariant::Type::DateTime)
    {
      str = m_value.toDateTime().toString("yyyy-MM-dd HH:mm:ss");
    }
    else
    {
      if(QVariant::Double == m_value.type() && 0 < m_precision)
      {
        str = QString::number(m_value.toDouble(),'f',m_precision);
      }
      else
      {
        if(m_value.canConvert<QString>())
          str = m_value.toString();
        else if(m_value.canConvert<QStringList>())
        {
          str = m_value.toStringList().join(",");
        }
        else
        {
          qCritical() << "value conversion to string may fail with type" << m_value.typeName();
        }
        if(str.contains(","))
        {
          QStringList values = str.split(",");
          if(Constants::DefaultSplitLength <= values.size())
          {
            QStringList list;
            for(int i=0;i<Constants::DefaultSplitLength;i++)
                list.push_back(values.at(i));

            str = list.join(",") + "...";
          }
          else
          {
            str = values.join(",");
          }
        }
      }
    }
    if(this->hasUnits())
    {
      return QString("%1 (%2)").arg(str,m_units);
    }
    else
    {
      return str;
    }
}
