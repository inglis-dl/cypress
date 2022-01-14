#include "TemplateTest.h"

// the minimum output data keys required from a successful a test
//
TemplateTest::TemplateTest()
{
    m_outputKeyList << "barcode";
}

void TemplateTest::fromFile(const QString& filePath)
{
}

// String representation for debug and GUI display purposes
//
QString TemplateTest::toString() const
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

bool TemplateTest::isValid() const
{
    return true;
}

// String keys are converted to snake_case
//
QJsonObject TemplateTest::toJsonObject() const
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
