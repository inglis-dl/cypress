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
    TestBase()
    {
        m_maximumNumberOfMeasurements = 1000;
        m_measurementSystem = "metric";
    }
    virtual ~TestBase() = default;

    // When a fixed maximum number of measurements is
    // established, the newest elements are kept and
    // the older ones are removed from the list
    //

    // String representation for debug and GUI display purposes
    //
    virtual QString toString() const = 0;

    virtual bool isValid() const = 0;

    // String keys are converted to snake_case
    //
    virtual QJsonObject toJsonObject() const = 0;

    virtual void reset();

    void setMeasurementSystem(const QString &s)
    {
      m_measurementSystem = "imperial" == s ? s : "metric";
    }

    QString getMeasurementSystem() const
    {
      return m_measurementSystem;
    }

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

    T getMeasurement(const quint32 &) const;

    T* find_first(const QString& key, const QVariant& value);

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
    MeasurementBase m_metaData;
    int m_maximumNumberOfMeasurements;
    QString m_measurementSystem;
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
T TestBase<T>::getMeasurement(const quint32 &i) const
{
    T m;
    if((int)i < m_measurementList.size())
      m = m_measurementList.at(i);
    return m;
}

template <class T>
T* TestBase<T>::find_first(const QString& key, const QVariant& value)
{
    for(auto&& x : m_measurementList)
    {
        for(auto&& v : x.toStdMap())
        {
            if(v.first==key && v.second==value)
            {
                return &x;
            }
        }
    }
    return nullptr;
}

#endif // TESTBASE_H
