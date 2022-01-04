#include "BodyCompositionTest.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringBuilder>

bool BodyCompositionTest::isValid() const
{
    bool okMeta =
      hasMetaDataCharacteristic("body type") &&
      hasMetaDataCharacteristic("gender") &&
      hasMetaDataCharacteristic("test datetime") &&
      hasMetaDataCharacteristic("age") &&
      hasMetaDataCharacteristic("height");

    bool okTest = 8 == getNumberOfMeasurements();
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
    return okMeta && okTest;
}

QString BodyCompositionTest::toString() const
{
    QString s;
    if(isValid())
    {
        QStringList l;
        l << QString("body type: ") % getMetaDataCharacteristic("body type").toString();
        l << QString("gender: ") % getMetaDataCharacteristic("gender").toString();
        l << QString("test datetime: ") % getMetaDataCharacteristic("test datetime").toDateTime().toString("yyyy-MM-dd hh:mm:ss");
        l << QString("age: ") % getMetaDataCharacteristic("age").toString();
        l << QString("height: ") % getMetaDataCharacteristic("height").toString();
        for(auto&& x : m_measurementList)
        {
            l << x.toString();
        }
        s = l.join("\n");
    }
    return s;
}

// The TanitaManager class provides the data after validating
// it via hasEndCode(arr) before passing to this class
//
void BodyCompositionTest::fromArray(const QByteArray &arr)
{
    if(!arr.isEmpty())
    {
      reset();
      m_array = arr;

      // NOTE: input body type can be overridden by the analyzer
      // depending on age and gender.  The body type used by
      // analyzer is recovered from the test output data here.
      //
      addMetaDataCharacteristic("body type", readBodyType());
      addMetaDataCharacteristic("gender", readGender());
      addMetaDataCharacteristic("test datetime", QDateTime::currentDateTime());
      addMetaDataCharacteristic("age", readAge());
      addMetaDataCharacteristic("height", readHeight());

      QString sys = getMeasurementSystem();

      BodyCompositionMeasurement m;
      m.setCharacteristic("weight", readWeight());
      m.setCharacteristic("units",("metric" == sys ? "kg" : "lb"));
      addMeasurement(m);

      m.reset();
      m.setCharacteristic("impedance", readImpedence());
      m.setCharacteristic("units","ohm");
      addMeasurement(m);

      m.reset();
      m.setCharacteristic("percent fat", readFatPercent());
      m.setCharacteristic("units","%");
      addMeasurement(m);

      m.reset();
      m.setCharacteristic("fat mass", readFatMass());
      m.setCharacteristic("units",("metric" == sys ? "kg" : "lb"));
      addMeasurement(m);

      m.reset();
      m.setCharacteristic("fat free mass", readFatFreeMass());
      m.setCharacteristic("units",("metric" == sys ? "kg" : "lb"));
      addMeasurement(m);

      m.reset();
      m.setCharacteristic("total body water", readTotalBodyWater());
      m.setCharacteristic("units",("metric" == sys ? "kg" : "lb"));
      addMeasurement(m);

      m.reset();
      m.setCharacteristic("body mass index", readBMI());
      m.setCharacteristic("units",("metric" == sys ? "kg/cm2" : "lb/in2"));
      addMeasurement(m);

      // TODO: check if bmr is always givin in kJ
      // conversions:
      // 1 kcal = 4.187 kJ
      // 1 kJ = 0.2388 kcal
      //
      m.reset();
      m.setCharacteristic("basal metabolic rate", readBMR());
      m.setCharacteristic("units","kJ");
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
    float result = s.toFloat(&ok);
    return ok ? QVariant(result) : QVariant();
}

QVariant BodyCompositionTest::readWeight() const
{
    QString s = readArray(10,14).trimmed();
    QVariant result;
    if(!s.isEmpty())
    {
      bool ok;
      float value = s.toFloat(&ok);
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
    float result = s.toFloat(&ok);
    return ok ? QVariant(result) : QVariant();
}

QVariant BodyCompositionTest::readFatPercent() const
{
    QString s = readArray(20,23).trimmed();
    QVariant result;
    if(!s.isEmpty())
    {
      bool ok;
      float value = s.toFloat(&ok);
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
      float value = s.toFloat(&ok);
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
      float value = s.toFloat(&ok);
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
      float value = s.toFloat(&ok);
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
      float value = s.toFloat(&ok);
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
      float value = s.toFloat(&ok);
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
