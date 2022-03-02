#include "TrueFlowSpirometerTest.h"

// the minimum output data keys required from a successful a test
//
TrueFlowSpirometerTest::TrueFlowSpirometerTest()
{
    m_outputKeyList << "barcode";
}

void TrueFlowSpirometerTest::fromFile(const QString& filePath)
{
}

// String representation for debug and GUI display purposes
//
QString TrueFlowSpirometerTest::toString() const
{
    QString displayString;
    if (isValid())
    {
        QStringList stringList;
        for (auto&& measurement : m_measurementList)
        {
            stringList << measurement.toString();
        }
        displayString = stringList.join("\n");
    }
    return displayString;
}

bool TrueFlowSpirometerTest::isValid() const
{
    return true;
}

// String keys are converted to snake_case
//
QJsonObject TrueFlowSpirometerTest::toJsonObject() const
{
    QJsonArray jsonArr;
    for (auto&& measurement : m_measurementList)
    {
        jsonArr.append(measurement.toJsonObject());
    }
    QJsonObject json;
    json.insert("test_meta_data", m_metaData.toJsonObject());
    json.insert("test_results", jsonArr);
    return json;
}
