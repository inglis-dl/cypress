#ifndef TESTBASE_H
#define TESTBASE_H

#include "MeasurementBase.h"

#include <QList>

/*!
 * \class TestBase
 * \brief An abstract Test class templated over measurement type
 *
 * Tests are comprised of instrument characteristics, or meta data,
 * and measurements.  Template specialization is done on the
 * measurement class parameter type, such as a HearingMeasurement.
 *
 * Inherited classes must implement the pure virtual methods:
 * toString, isValid, toJsonObject
 *
 * \sa MeasurementBase, AutiometerTest, QList
 *
 */

QT_FORWARD_DECLARE_CLASS(QJsonObject)

template <class T>
class TestBase
{
public:
    TestBase() = default;
    virtual ~TestBase() = default;

    virtual QString toString() const = 0;

    virtual bool isValid() const = 0;

    virtual QJsonObject toJsonObject() const = 0;

    void reset();

    void setMetataData(const MeasurementBase &other)
    {
        m_metaData = other;
    }

    MeasurementBase getMetaData() const
    {
        return m_metaData;
    }

    void addMetaDataCharacteristic(const QString &key, const QVariant &value)
    {
        m_metaData.setCharacteristic(key,value);
    }

    QVariant getMetaDataCharacteristic(const QString &key) const
    {
        return m_metaData.getCharacteristic(key);
    }

    bool hasMetaDataCharacteristic(const QString &key) const
    {
        return m_metaData.hasCharacteristic(key);
    }

    void addMeasurement(const T &);

    void pop_front()
    {
        m_measurementList.pop_front();
    }

    void pop_back()
    {
        m_measurementList.pop_back();
    }

    int getNumberOfMeasurements() const
    {
        return m_measurementList.size();
    }

protected:
    QList<T> m_measurementList;

private:
    MeasurementBase m_metaData;
};

template <class T>
void TestBase<T>::reset()
{
    m_metaData.reset();
    m_measurementList.clear();
}

template <class T>
void TestBase<T>::addMeasurement(const T &item)
{
    if(!m_measurementList.contains(item))
      m_measurementList.append(item);
}

#endif // TESTBASE_H
