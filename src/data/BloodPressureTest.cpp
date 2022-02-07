#include "BloodPressureTest.h"

// the minimum output data keys required from a successful a test
//
BloodPressureTest::BloodPressureTest()
{
    m_outputKeyList << "user id";
}

void BloodPressureTest::addMeasurement(const int sbp, const int dbp, const int pulse, const bool isAverage)
{
    BloodPressureMeasurement measurement;
    measurement.storeData(sbp, dbp, pulse, isAverage);
    m_measurementList.append(measurement);
}

// String representation for debug and GUI display purposes
//
QString BloodPressureTest::toString() const
{
    QString outputStr;
    if (isValid())
    {
        QStringList tempList;
        for (auto&& measurement : m_measurementList)
        {
            tempList << measurement.toString();
        }
        outputStr = tempList.join("\n");
    }
    return outputStr;
}

bool BloodPressureTest::isValid() const
{
    for (auto&& measurement : m_measurementList)
    {
        if (measurement.isValid() == false) {
            return false;
        }
    }
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
