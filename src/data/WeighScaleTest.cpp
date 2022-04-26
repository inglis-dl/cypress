#include "WeighScaleTest.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

bool WeighScaleTest::isValid() const
{
    //TODO: handle partial cases wherein we want to update the UI
    // with tests containing the minimum measurement count but
    // not the full expected count required for writing
    //
    bool okTest = (getMeasurementCount() == getExpectedMeasurementCount()) ||
                  (0 < getMeasurementCount());
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

QString WeighScaleTest::toString() const
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
         bool ok = true;
         if(0 < getMeasurementCount())
         {
           WeightMeasurement last = lastMeasurement();
           QDateTime prev = last.getAttributeValue("timestamp").toDateTime();
           QDateTime curr = m.getAttributeValue("timestamp").toDateTime();
           ok =  DELAY < prev.secsTo(curr);
         }
         if(ok)
           addMeasurement(m);
       }
    }
}

QJsonObject WeighScaleTest::toJsonObject() const
{
    QJsonArray jsonArr;
    foreach(const auto m, m_measurementList)
    {
      jsonArr.append(m.toJsonObject());
    }
    QJsonObject json;
    if(!metaDataIsEmpty())
    {
      json.insert("test_meta_data",m_metaData.toJsonObject());
    }
    json.insert("test_results",jsonArr);
    return json;
}
