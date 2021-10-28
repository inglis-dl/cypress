#ifndef MEASUREMENTBASE_H
#define MEASUREMENTBASE_H

#include <QMap>
#include <QVariant>

/*!
 * \class MeasurementBase
 * \brief A Base Measurement class
 *
 * Measurements are composed of atomic elements or "characteristics".
 * This class facilitates parsing measurement data from different
 * instruments into component characteristics, such as value, units etc.
 * and stores them labeled by QString keys as QVariant object values in a QMap.
 * The measurement can be retrieved as a QString for UI purposes.
 *
 * \note  This class stores one or more characteristics which taken as whole
 * can represent a single measurement.  A Test class inherited from TestBase
 * should be used to store multiple measurements.
 *
 * Child classes should override toString and isValid methods as appropriate.
 *
 * This class can also be used to store instrument characteristics such as
 * serial number, model number etc.
 *
 * \sa QVariant, QString, QMap
 *
 */

class MeasurementBase
{
public:
    MeasurementBase() = default;
    ~MeasurementBase() = default;
    MeasurementBase(const MeasurementBase &other)
    {
        m_characteristicValues = other.m_characteristicValues;
    }
    MeasurementBase operator=(const MeasurementBase &other)
    {
        m_characteristicValues = other.m_characteristicValues;
        return *this;
    }

    // String representation for debug and GUI display purposes
    //
    virtual QString toString() const;

    virtual bool isValid() const;

    // String keys are converted to snake_case
    //
    virtual QJsonObject toJsonObject() const;

    void reset();

    void setCharacteristic(const QString &key, const QVariant &value)
    {
      m_characteristicValues[key]=value;
    }

    inline QVariant getCharacteristic(const QString &key) const
    {
        return m_characteristicValues.contains(key) ?
               m_characteristicValues[key] : QVariant();
    }

    QMap<QString,QVariant> getCharacteristicValues() const
    {
        return m_characteristicValues;
    }

    inline bool hasCharacteristic(const QString &key) const
    {
        return m_characteristicValues.contains(key) &&
               !m_characteristicValues[key].isNull() &&
                m_characteristicValues[key].isValid();
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

QDebug operator<<(QDebug dbg, const MeasurementBase &);

#endif // MEASUREMENTBASE_H
