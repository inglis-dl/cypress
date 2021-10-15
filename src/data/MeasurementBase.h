#ifndef MEASUREMENTBASE_H
#define MEASUREMENTBASE_H

#include <QMap>
#include <QVariant>

/*!
 * \class MeasurementBase
 * \brief A Base Measurement class
 *
 * Measurements of are composed of atomic elements or "characteristics".
 * This class facilitates parsing measurement data from different
 * instruments into component characteristics, such as value, units etc.
 * and stores them labeled by QString keys as QVariant object values in a QMap.
 * The measurement can be retrieved as a QString for UI purposes.
 *
 * \note  This class stores one measurement only.  A manager
 * class should be used to store multiple measurements.  Child classes
 * must implement toString and isValid methods.
 *
 * \sa QVariant, QString, QMap
 *
 */

class MeasurementBase
{
public:
    MeasurementBase() = default;
    ~MeasurementBase() = default;
    MeasurementBase(const MeasurementBase &);
    MeasurementBase(const QString &key, const QVariant &value)
    {
        m_characteristicValues[key]=value;
    }
    MeasurementBase &operator=(const MeasurementBase &);

    virtual QString toString() const;
    virtual bool isValid() const;

    void reset();

    void setCharacteristic(const QString &key, const QVariant &value)
    {
      m_characteristicValues[key]=value;
    }

    QVariant getCharacteristic(const QString &key) const
    {
        return m_characteristicValues.contains(key) ?
               m_characteristicValues[key] : QVariant();
    }

    QMap<QString,QVariant> getCharacteristicValues() const
    {
        return m_characteristicValues;
    }

protected:

    QMap<QString,QVariant> m_characteristicValues;
};

Q_DECLARE_METATYPE(MeasurementBase);

inline bool operator==(const MeasurementBase &lhs, const MeasurementBase &rhs)
{
    return  lhs.getCharacteristicValues()==rhs.getCharacteristicValues();
}
inline bool operator!=(const MeasurementBase &lhs, const MeasurementBase &rhs)
{
    return !(lhs == rhs);
}
QDebug operator<<(QDebug dbg, const MeasurementBase &measurement);

#endif // MEASUREMENTBASE_H
