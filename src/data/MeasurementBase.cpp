#include "MeasurementBase.h"

#include <QDebug>

MeasurementBase::MeasurementBase(const MeasurementBase &other)
{
    m_characteristicValues = other.m_characteristicValues;
}

MeasurementBase& MeasurementBase::operator=(const MeasurementBase &other)
{
    m_characteristicValues = other.m_characteristicValues;
    return *this;
}

bool MeasurementBase::isValid() const
{
    qDebug() << "in measurement base isValid";
    bool ok = true;
    for( auto it = m_characteristicValues.constBegin(),
         end = m_characteristicValues.constEnd(); it != end; ++it)
    {
        if(it.value().isNull())
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
    qDebug() << "in measurement base toString building with a string list";
    QStringList list;
    for( auto it = m_characteristicValues.constBegin(),
         end = m_characteristicValues.constEnd(); it != end; ++it)
    {
        list << it.value().toString();
    }
    qDebug() << "in measurement base toString returning a joined string list";
    return list.join(" ");
}

QDebug operator<<(QDebug dbg, const MeasurementBase &measurement)
{
    const QString output = measurement.toString();
    if (output.isEmpty())
        dbg.nospace() << "Measurement()";
    else
        dbg.nospace() << "Measurement(" << output << " ...)";
    return dbg.maybeSpace();
}

