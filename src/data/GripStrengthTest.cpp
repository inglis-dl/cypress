#include "GripStrengthTest.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

// the minimum output data keys required from a successful a test
//
GripStrengthTest::GripStrengthTest()
{
    // if a device returns a patient identifier for example:
    // m_outputKeyList << "patient_id";
}

// String representation for debug and GUI display purposes
//
QString GripStrengthTest::toString() const
{
    QString str;
    if(isValid())
    {
      QStringList list;
      foreach(auto measurement, m_measurementList)
      {
        list << measurement.toString();
      }
      str = list.join("\n");
    }
    return str;
}

bool GripStrengthTest::isValid() const
{
    bool okMeta = true;
    foreach(auto key, m_outputKeyList)
    {
      if(!hasMetaData(key))
      {
         okMeta = false;
         break;
       }
    }
    bool okTest = getMeasurementCount() == getExpectedMeasurementCount();
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
    return okMeta && okTest;
}

QJsonObject GripStrengthTest::toJsonObject() const
{
    QJsonArray jsonArr;
    foreach(auto m, m_measurementList)
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
