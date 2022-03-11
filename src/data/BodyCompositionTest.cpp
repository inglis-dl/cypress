#include "BodyCompositionTest.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringBuilder>

BodyCompositionTest::BodyCompositionTest()
{
    m_outputKeyList << "body_type";
    m_outputKeyList << "gender";
    m_outputKeyList << "test_datetime";
    m_outputKeyList << "age";
    m_outputKeyList << "height";
}

bool BodyCompositionTest::isValid() const
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
    bool okTest = 1 == getNumberOfMeasurements();
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

QStringList BodyCompositionTest::toStringList() const
{
  QStringList list;
  if(isValid())
  {
      list << getMetaData("body_type").toString();
      list << getMetaData("gender").toString();
      list << getMetaData("test_datetime").toString();
      list << getMetaData("age").toString();
      list << getMetaData("height").toString();
      auto m = getMeasurement(0);
      foreach(auto att, m.getAttributes())
      {
        list << m.toString();
      }
  }
  return list;
}

QString BodyCompositionTest::toString() const
{
    QString str;
    if(isValid())
    {
      str = toStringList().join("\n");
    }
    return str;
}

// The manager class provides the data after validating
// it via hasEndCode(arr) before passing to this class
//
void BodyCompositionTest::fromArray(const QByteArray &arr)
{
    if(!arr.isEmpty())
    {
      reset();
      m_array = arr;
      Constants::UnitsSystem unitsSystem = getUnitsSystem();
      QString sys = Constants::getUnitsSystemName(unitsSystem);

      // NOTE: input body type can be overridden by the analyzer
      // depending on age and gender.  The body type used by
      // analyzer is recovered from the test output data here.
      //
      addMetaData("test_datetime", QDateTime::currentDateTime());
      addMetaData("body_type", readBodyType());
      addMetaData("gender", readGender());
      addMetaData("age", readAge());

      QString units = "metric" == sys ? "cm" : "in";
      addMetaData("height", readHeight(), units);

      BodyCompositionMeasurement m;
      units = "metric" == sys ? "kg" : "lb";
      m.setAttribute("weight", readWeight(), units);
      m.setAttribute("impedance", readImpedence(), "ohm");
      m.setAttribute("percent_fat", readFatPercent(), "%");
      m.setAttribute("fat_mass", readFatMass(), units);
      m.setAttribute("fat_free_mass", readFatFreeMass(), units);
      m.setAttribute("total_body_water", readTotalBodyWater(), units);
      units = "metric" == sys ? "kg/cm2" : "lb/in2";
      m.setAttribute("body_mass_index", readBMI(), units);

      // TODO: check if bmr is always givin in kJ
      // conversions:
      // 1 kcal = 4.187 kJ
      // 1 kJ = 0.2388 kcal
      //
      m.setAttribute("basal_metabolic_rate", readBMR(), "kJ");
      addMeasurement(m);
    }
}

QString BodyCompositionTest::readArray(const quint8 &begin, const quint8 &end) const
{
    int len = end-begin+1;
    return 0<len && end<m_array.size() ? QString::fromLatin1(m_array.mid(begin, len)) : QString();
}

QString BodyCompositionTest::readBodyType() const
{
   QString s = readArray(0,0);
   return "0" == s ? "standard" : ("2" == s ? "athletic" : "unknown");
}

QString BodyCompositionTest::readGender() const
{
    QString s = readArray(2,2);
    return "1" == s ? "male" : ("2" == s ? "female" : "unknown");
}

QVariant BodyCompositionTest::readHeight() const
{
    QString s = readArray(4,8).trimmed();
    bool ok;
    float result = s.toDouble(&ok);
    return ok ? QVariant(result) : QVariant();
}

QVariant BodyCompositionTest::readWeight() const
{
    QString s = readArray(10,14).trimmed();
    QVariant result;
    if(!s.isEmpty())
    {
      bool ok;
      float value = s.toDouble(&ok);
      if(ok)
      {
        char buffer[10];
        sprintf(buffer,"%5.1f",value);
        result = QString::fromLatin1(buffer).toDouble();
      }
    }
    return result;
}

QVariant BodyCompositionTest::readImpedence() const
{
    QString s = readArray(16,18).trimmed();
    bool ok;
    float result = s.toDouble(&ok);
    return ok ? QVariant(result) : QVariant();
}

QVariant BodyCompositionTest::readFatPercent() const
{
    QString s = readArray(20,23).trimmed();
    QVariant result;
    if(!s.isEmpty())
    {
      bool ok;
      float value = s.toDouble(&ok);
      if(ok)
      {
        char buffer[10];
        sprintf(buffer,"%5.1f",value);
        result = QString::fromLatin1(buffer).toDouble();
      }
    }
    return result;
}

QVariant BodyCompositionTest::readFatMass() const
{
    QString s = readArray(25,29).trimmed();
    QVariant result;
    if(!s.isEmpty())
    {
      bool ok;
      float value = s.toDouble(&ok);
      if(ok)
      {
        char buffer[10];
        sprintf(buffer,"%5.1f",value);
        result = QString::fromLatin1(buffer).toDouble();
      }
    }
    return result;
}

QVariant BodyCompositionTest::readFatFreeMass() const
{
    QString s = readArray(31,35).trimmed();
    QVariant result;
    if(!s.isEmpty())
    {
      bool ok;
      float value = s.toDouble(&ok);
      if(ok)
      {
        char buffer[10];
        sprintf(buffer,"%5.1f",value);
        result = QString::fromLatin1(buffer).toDouble();
      }
    }
    return result;
}

QVariant BodyCompositionTest::readTotalBodyWater() const
{
    QString s = readArray(37,41).trimmed();
    QVariant result;
    if(!s.isEmpty())
    {
      bool ok;
      float value = s.toDouble(&ok);
      if(ok)
      {
        char buffer[10];
        sprintf(buffer,"%5.1f",value);
        result = QString::fromLatin1(buffer).toDouble();
      }
    }
    return result;
}

QVariant BodyCompositionTest::readAge() const
{
    QString s = readArray(43,44).trimmed();
    QVariant result;
    if(!s.isEmpty())
    {
      result = s.toUInt();
    }
    return result;
}

QVariant BodyCompositionTest::readBMI() const
{
    QString s = readArray(46,50).trimmed();
    QVariant result;
    if(!s.isEmpty())
    {
      bool ok;
      float value = s.toDouble(&ok);
      if(ok)
      {
        char buffer[10];
        sprintf(buffer,"%5.1f",value);
        result = QString::fromLatin1(buffer).toDouble();
      }
    }
    return result;
}

QVariant BodyCompositionTest::readBMR() const
{
    QString s = readArray(52,56).trimmed();
    QVariant result;
    if(!s.isEmpty())
    {
      bool ok;
      float value = s.toDouble(&ok);
      if(ok)
      {
        char buffer[10];
        sprintf(buffer,"%5.1f",value);
        result = QString::fromLatin1(buffer).toDouble();
      }
    }
    return result;
}

QJsonObject BodyCompositionTest::toJsonObject() const
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
