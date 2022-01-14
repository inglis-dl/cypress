#include "BloodPressureTest.h"

// the minimum output data keys required from a successful a test
//
BloodPressureTest::BloodPressureTest()
{
    m_outputKeyList << "user id";
}

void BloodPressureTest::fromFile(const QString& filePath)
{
}

// String representation for debug and GUI display purposes
//
QString BloodPressureTest::toString() const
{
    QString s;
    if (isValid())
    {
        QStringList l;
        for (auto&& x : m_measurementList)
        {
            l << x.toString();
        }
        s = l.join("\n");
    }
    return s;
}

bool BloodPressureTest::isValid() const
{
    return true;
}

// String keys are converted to snake_case
//
QJsonObject BloodPressureTest::toJsonObject() const
{
    QJsonArray jsonArr;
    for (auto&& x : m_measurementList)
    {
        jsonArr.append(x.toJsonObject());
    }
    QJsonObject json;
    json.insert("test_meta_data", m_metaData.toJsonObject());
    json.insert("test_results", jsonArr);
    return json;
}
