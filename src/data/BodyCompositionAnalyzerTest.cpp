#include "BodyCompositionAnalyzerTest.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringBuilder>

bool BodyCompositionAnalyzerTest::isValid() const
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

QString BodyCompositionAnalyzerTest::toString() const
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
void BodyCompositionAnalyzerTest::fromArray(const QByteArray &arr)
{
    if(!arr.isEmpty())
    {
      reset();
      m_array = arr;

      addMetaDataCharacteristic("body type", readBodyType());
      addMetaDataCharacteristic("gender", readGender());
      addMetaDataCharacteristic("test datetime", QDateTime::currentDateTime());
      addMetaDataCharacteristic("age", readAge());
      addMetaDataCharacteristic("height", readHeight());

      BodyCompositionMeasurement m;
      m.setCharacteristic("weight", readWeight());
      m.setCharacteristic("units","kg");
      addMeasurement(m);

      m.reset();
      m.setCharacteristic("impedence", readImpedence());
      m.setCharacteristic("units","ohm");
      addMeasurement(m);

      m.reset();
      m.setCharacteristic("percent fat", readFatPercent());
      m.setCharacteristic("units","%");
      addMeasurement(m);

      m.reset();
      m.setCharacteristic("fat mass", readFatMass());
      m.setCharacteristic("units","kg");
      addMeasurement(m);

      m.reset();
      m.setCharacteristic("fat free mass", readFatFreeMass());
      m.setCharacteristic("units","kg");
      addMeasurement(m);

      m.reset();
      m.setCharacteristic("total body water", readTotalBodyWater());
      m.setCharacteristic("units","kg");
      addMeasurement(m);

      m.reset();
      m.setCharacteristic("body mass index", readBMI());
      m.setCharacteristic("units","kg/cm2");
      addMeasurement(m);

      m.reset();
      m.setCharacteristic("bmr", readBMR());
      m.setCharacteristic("units","kJ");
      addMeasurement(m);
    }
}

QString BodyCompositionAnalyzerTest::readArray(const quint8 &begin, const quint8 &end) const
{
    int len = end-begin+1;
    return 0<len && end<m_array.size() ? QString::fromLatin1(m_array.mid(begin, len)) : QString();
}

QString BodyCompositionAnalyzerTest::readBodyType() const
{
   QString s = readArray(0,0);
   return "0" == s ? "standard" : ("2" == s ? "athletic" : "unknown");
}

QString BodyCompositionAnalyzerTest::readGender() const
{
    QString s = readArray(2,2);
    return "1" == s ? "male" : ("2" == s ? "female" : "unknown");
}

QVariant BodyCompositionAnalyzerTest::readHeight() const
{
    QString s = readArray(4,8).trimmed();
    bool ok;
    float result = s.toFloat(&ok);
    return ok ? QVariant(result) : QVariant();
}

QVariant BodyCompositionAnalyzerTest::readWeight() const
{
    QString s = readArray(10,14).trimmed();
    bool ok;
    float result = s.toFloat(&ok);
    return ok ? QVariant(result) : QVariant();
}

QVariant BodyCompositionAnalyzerTest::readImpedence() const
{
    QString s = readArray(16,18).trimmed();
    bool ok;
    float result = s.toFloat(&ok);
    return ok ? QVariant(result) : QVariant();
}

QVariant BodyCompositionAnalyzerTest::readFatPercent() const
{
    QString s = readArray(20,23).trimmed();
    bool ok;
    float result = s.toFloat(&ok);
    return ok ? QVariant(result) : QVariant();
}

QVariant BodyCompositionAnalyzerTest::readFatMass() const
{
    QString s = readArray(25,29).trimmed();
    bool ok;
    float result = s.toFloat(&ok);
    return ok ? QVariant(result) : QVariant();
}

QVariant BodyCompositionAnalyzerTest::readFatFreeMass() const
{
    QString s = readArray(31,35).trimmed();
    bool ok;
    float result = s.toFloat(&ok);
    return ok ? QVariant(result) : QVariant();
}

QVariant BodyCompositionAnalyzerTest::readTotalBodyWater() const
{
    QString s = readArray(37,41).trimmed();
    bool ok;
    float result = s.toFloat(&ok);
    return ok ? QVariant(result) : QVariant();
}

QVariant BodyCompositionAnalyzerTest::readAge() const
{
    QString s = readArray(43,44).trimmed();
    bool ok;
    float result = s.toFloat(&ok);
    return ok ? QVariant(result) : QVariant();
}

QVariant BodyCompositionAnalyzerTest::readBMI() const
{
    QString s = readArray(46,50).trimmed();
    bool ok;
    float result = s.toFloat(&ok);
    return ok ? QVariant(result) : QVariant();
}

QVariant BodyCompositionAnalyzerTest::readBMR() const
{
    QString s = readArray(52,56).trimmed();
    bool ok;
    float result = s.toFloat(&ok);
    return ok ? QVariant(result) : QVariant();
}

QJsonObject BodyCompositionAnalyzerTest::toJsonObject() const
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
