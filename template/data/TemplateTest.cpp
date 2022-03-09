#include "TemplateTest.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

// the minimum output data keys required from a successful a test
//
TemplateTest::TemplateTest()
{
    // if a device returns a patient identifier for example:
    // m_outputKeyList << "patient_id";
}

// String representation for debug and GUI display purposes
//
QString TemplateTest::toString() const
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

bool TemplateTest::isValid() const
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
    bool okTest = 0 < getNumberOfMeasurements();
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

QJsonObject TemplateTest::toJsonObject() const
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
