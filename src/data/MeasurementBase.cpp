#include "MeasurementBase.h"

#include <QDebug>

bool MeasurementBase::isValid() const
{
    bool ok = true;
    for(auto&& x : m_characteristicValues)
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
    for(auto&& x :  m_characteristicValues)
    {
      l << x.toString();
    }
    return l.join(" ");
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
