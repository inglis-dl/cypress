#ifndef TESTBASE_H
#define TESTBASE_H

#include "Measurement.h"
#include "../auxiliary/Constants.h"

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
 * \sa Measurement, AudiometerTest, QList
 *
 */

QT_FORWARD_DECLARE_CLASS(QJsonObject)

template <class T>
class TestBase
{
public:
    TestBase() = default;
    virtual ~TestBase() = default;

    // When a fixed maximum number of measurements is
    // established, the newest elements are kept and
    // the older ones are removed from the list
    //

    // TODO: implement validation for tests that capture participant ID meta data
    // against an interview barcode
    //

    // String representation for debug and GUI display purposes
    //
    virtual QString toString() const = 0;

    virtual bool isValid() const = 0;

    // String keys are converted to snake_case
    //
    virtual QJsonObject toJsonObject() const = 0;

    // default is to reset both meta and measurement data
    //
    virtual void reset();

    void setUnitsSystem(const Constants::UnitsSystem& system)
    {
      m_unitsSystem = system;
    }

    Constants::UnitsSystem getUnitsSystem() const
    {
      return m_unitsSystem;
    }

    void setMetataData(const Measurement &other)
    {
      m_metaData = other;
    }

    Measurement getMetaData() const
    {
      return m_metaData;
    }

    void addMetaData(const QString& key, const QVariant& value, const QString& units)
    {
      m_metaData.setAttribute(key, value, units);
    }

    void addMetaData(const QString& key, const QVariant& value, const int& precision)
    {
      m_metaData.setAttribute(key, value, QString(), precision);
    }

    void addMetaData(const QString& key, const QVariant& value)
    {
      m_metaData.setAttribute(key, value);
    }

    void addMetaData(const QString &key, const Measurement::Value &value)
    {
      m_metaData.setAttribute(key, value);
    }

    QVariant getMetaData(const QString &key) const
    {
      return m_metaData.getAttributeValue(key);
    }

    QString getMetaDataAsString(const QString &key) const
    {
      return m_metaData.getAttribute(key).toString();
    }

    bool hasMetaData() const
    {
      return !m_metaData.isEmpty();
    }

    bool hasMetaData(const QString &key) const
    {
      return m_metaData.hasAttribute(key);
    }

    void addMeasurement(const T&);

    bool hasMeasurement(const int&) const;

    T getMeasurement(const int&) const;

    int getNumberOfMeasurements() const
    {
      return m_measurementList.size();
    }

    void setMaximumNumberOfMeasurements(const int& max)
    {
      m_maximumNumberOfMeasurements = 0 < max ? max : 1;
    }

    int getMaximumNumberOfMeasurements() const
    {
      return m_maximumNumberOfMeasurements;
    }

protected:
    QVector<T> m_measurementList;
    Measurement m_metaData;
    int m_maximumNumberOfMeasurements { 1000 };
    Constants::UnitsSystem m_unitsSystem { Constants::UnitsSystem::systemMetric };
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
    {
        m_measurementList.append(item);
        if(m_maximumNumberOfMeasurements < m_measurementList.size())
            m_measurementList.pop_front();
    }
}

template <class T>
T TestBase<T>::getMeasurement(const int &index) const
{
    T m;
    if(index < m_measurementList.size())
      m = m_measurementList.at(index);
    return m;
}

template <class T>
bool TestBase<T>::hasMeasurement(const int& index) const
{
    T m = getMeasurement(index);
    return !m.isEmpty();
}

#endif // TESTBASE_H
