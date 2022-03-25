#include "TemperatureTest.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>

bool TemperatureTest::isValid() const
{
    bool okTest = getNumberOfMeasurements() == getMaximumNumberOfMeasurements();
    if(okTest)
    {
      foreach(const auto m, m_measurementList)
      {
        if(!m.isValid())
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
    QString str;
    if(isValid())
    {
      QStringList list;
      foreach(const auto m, m_measurementList)
      {
        list << m.toString();
      }
      str = list.join("\n");
    }
    return str;
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
    foreach(const auto m, m_measurementList)
    {
      jsonArr.append(m.toJsonObject());
    }
    QJsonObject json;
    if(hasMetaData())
      json.insert("test_meta_data",m_metaData.toJsonObject());
    json.insert("test_results",jsonArr);
    return json;
}
