#include "WeighScaleTest.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

bool WeighScaleTest::isValid() const
{
    bool okTest = getNumberOfMeasurements() == getMaximumNumberOfMeasurements();
    if(okTest)
    {
      foreach(auto m, m_measurementList)
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

QString WeighScaleTest::toString() const
{
    QString str;
    if(isValid())
    {
      QStringList list;
      foreach(auto m, m_measurementList)
      {
        list << m.toString();
      }
      str = list.join("\n");
    }
    return str;
}

void WeighScaleTest::fromArray(const QByteArray &arr)
{
    if(!arr.isEmpty())
    {
       // only add to the end and keep the last two tests
       //
       WeightMeasurement m;
       m.fromArray(arr);
       if(m.isValid())
       {
         addMeasurement(m);
       }
    }
}

QJsonObject WeighScaleTest::toJsonObject() const
{
    QJsonArray jsonArr;
    foreach(auto m, m_measurementList)
    {
      jsonArr.append(m.toJsonObject());
    }
    QJsonObject json;
    if(hasMetaData())
      json.insert("test_meta_data",m_metaData.toJsonObject());
    json.insert("test_results",jsonArr);
    return json;
}
