#include "TemperatureTest.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

bool TemperatureTest::isValid() const
{
    bool okTest = getNumberOfMeasurements() == getMaximumNumberOfMeasurements();
    if(okTest)
    {
      for(auto&& x : m_measurementList)
      {
        if(!x.isValid())
        {
          okTest = false;
          break;
        }
      }
    }
    return okTest;
}

QString TemperatureTest::toString() const
{
    QString s;
    if(isValid())
    {
        QStringList l;
        for(auto&& x : m_measurementList)
        {
            l << x.toString();
        }
        s = l.join("\n");
    }
    return s;
}

void TemperatureTest::fromArray(const QByteArray &arr)
{
    if(!arr.isEmpty())
    {
       // only add to the end and keep the last two tests
       //
       TemperatureMeasurement m;
       m.fromArray(arr);
       if(m.isValid())
       {
         addMeasurement(m);
       }
    }
}

QJsonObject TemperatureTest::toJsonObject() const
{
    QJsonArray jsonArr;
    for(auto&& x : m_measurementList)
    {
        QJsonObject test = x.toJsonObject();
        jsonArr.append(test);
    }
    QJsonObject json;
    json.insert("test_meta_data",m_metaData.toJsonObject());
    json.insert("test_results",jsonArr);
    return json;
}
