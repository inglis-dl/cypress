#include "TestBase.h"

#include <QJsonArray>

TestBase::TestBase(const TestBase &other)
{
    m_measurementList = other.m_measurementList;
    m_metadata = other.m_metadata;
}

TestBase& TestBase::operator=(const TestBase &other)
{
    m_measurementList = other.m_measurementList;
    m_metadata = other.m_metadata;
    return *this;
}

void TestBase::addMeasurement(const MeasurementBase &item)
{
    if(!m_measurementList.contains(item))
        m_measurementList.append(item);
}

void TestBase::reset()
{
    m_measurementList.clear();
    m_metadata.reset();
}

bool TestBase::isValid() const
{
    bool ok = true;
    return ok;
}

QString TestBase::toString() const
{
    return QString();
}

QJsonObject TestBase::toJsonObject() const
{
    QJsonObject json;
//    json.insert("meta",QJsonValue(jsonObjMeta));
//    json.insert("measurement",QJsonValue(jsonObjMeasurement));

    return json;
}
