#include "WeighScaleTest.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

bool WeighScaleTest::isValid() const
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

QString WeighScaleTest::toString() const
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
