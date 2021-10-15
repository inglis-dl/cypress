#ifndef TESTBASE_H
#define TESTBASE_H

#include <QJsonObject>
#include <QList>
#include "MeasurementBase.h"

class TestBase
{
public:
    TestBase() = default;
    ~TestBase() = default;
    TestBase(const TestBase &);
    virtual TestBase &operator=(const TestBase &);

    virtual QString toString() const;

    virtual bool isValid() const;

    virtual void reset();

    void addMeasurement(const MeasurementBase &);
    void setMetatadata(const MeasurementBase &other)
    {
        m_metadata = other;
    }
    void addMetadata(const QString &key, const QVariant &value)
    {
        m_metadata.setCharacteristic(key,value);
    }
    QList<MeasurementBase> getMeasurementList()
    {
        return m_measurementList;
    }

    MeasurementBase getMetadata()
    {
        return m_metadata;
    }

    QJsonObject toJsonObject() const;

protected:
    QList<MeasurementBase> m_measurementList;
    MeasurementBase m_metadata;
};

Q_DECLARE_METATYPE(TestBase);

QDebug operator<<(QDebug dbg, const TestBase &);

#endif // TESTBASE_H
