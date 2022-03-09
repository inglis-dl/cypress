#include "MeasurementBase.h"

#include <QDebug>
#include <QJsonObject>

bool MeasurementBase::isValid() const
{
    bool ok = true;
    for(auto x, m_characteristicValues)
    {
      if(x.isNull())
      {
        ok = false;
        break;
      }
    }
    return ok;
}

void MeasurementBase::reset()
{
    m_characteristicValues.clear();
}

QString MeasurementBase::toString() const
{
    QStringList l;
    for(auto x,  m_characteristicValues)
    {
      l << x.toString();
    }
    return l.join(" ");
}

QJsonObject MeasurementBase::toJsonObject() const
{
    QJsonObject json;
    for(auto x, m_characteristicValues.toStdMap())
    {
        // convert to space delimited phrases to snake_case
        //
        json.insert(QString(x.first).toLower().replace(QRegExp("[\\s]+"),"_"), QJsonValue::fromVariant(x.second));
    }
    return json;
}

QDebug operator<<(QDebug dbg, const MeasurementBase &item)
{
    const QString s = item.toString();
    if (s.isEmpty())
      dbg.nospace() << "Measurement()";
    else
      dbg.nospace() << "Measurement(" << s << " ...)";
    return dbg.maybeSpace();
}
