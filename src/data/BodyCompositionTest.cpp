#include "BodyCompositionTest.h"

#include "../auxiliary/Utilities.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
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
    foreach(const auto key, m_outputKeyList)
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
      foreach(const auto m, m_measurementList)
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
      list << QString("body type: %1").arg(getMetaDataAsString("body_type"));
      list << QString("gender: %1").arg(getMetaData("gender").toString());
      list << QString("test datetime: %1").arg(getMetaDataAsString("test_datetime"));
      list << QString("age: %1").arg(getMetaDataAsString("age"));
      list << QString("height: %1").arg(getMetaDataAsString("height"));
      BodyCompositionMeasurement m = getMeasurement(0);
      list << QString("weight: %1").arg(m.getAttribute("weight").toString());
      list << QString("percent fat: %1").arg(m.getAttribute("percent_fat").toString());
      list << QString("fat mass: %1").arg(m.getAttribute("fat_mass").toString());
      list << QString("fat free mass: %1").arg(m.getAttribute("fat_free_mass").toString());
      list << QString("total body water: %1").arg(m.getAttribute("total_body_water").toString());
      list << QString("body mass index: %1").arg(m.getAttribute("body_mass_index").toString());
      list << QString("impedance: %1").arg(m.getAttribute("impedance").toString());
      list << QString("basal metabolic rate: %1").arg(m.getAttribute("basal_metabolic_rate").toString());
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

void BodyCompositionTest::simulate(
        const double& age,
        const QString& gender,
        double height)
{
    addMetaData("test_datetime", QDateTime::currentDateTime());
    addMetaData("body_type", "standard");
    addMetaData("gender", gender);
    addMetaData("age", age, "yr"); // integer

    QString sys = Constants::getUnitsSystemName(m_unitsSystem);
    QString units = "metric" == sys ? "cm" : "in";
    addMetaData("height", height, units);

    BodyCompositionMeasurement m;
    units = "metric" == sys ? "kg" : "lb";

    double mu = QRandomGenerator::global()->generateDouble();

    // BJ Devine formula 1974
    // get number of inches over 5'
    //
    double weight = ("male" == gender) ? 50.0f : 45.5f;
    double over = (("metric" == sys) ? height/2.54f : height ) - 60.0f;
    if(0.0 < over)
      weight += 2.3f*over;

    weight = ("metric" == sys) ? weight : (weight * 2.20462f);
    m.setAttribute("weight", weight, units);

    double impedance = QRandomGenerator::global()->bounded(150,900);
    m.setAttribute("impedance", impedance, "ohm");

    // possible range is gender and age dependent
    // for adults 40 - 60+ range is 14% - 35%
    //
    double pfat = Utilities::interp(14.0f,35.0f,mu);
    m.setAttribute("percent_fat", pfat, "%");
    double fmass = 0.01f * pfat * weight;
    m.setAttribute("fat_mass", fmass, units);
    double ffmass = weight - fmass;
    m.setAttribute("fat_free_mass", ffmass, units);

    // normal range for women varies betw 45% and 60%
    // normal range for men varies betw 50% and 65%
    //
    double wmass = weight * Utilities::interp(0.45f,0.65f,mu);
    m.setAttribute("total_body_water", wmass, units);

    units = "metric" == sys ? "kg/cm2" : "lb/in2";
    double bmi = weight / (height*height);
    m.setAttribute("body_mass_index", bmi, units);

    // TODO: check if bmr is always givin in kJ
    // conversions:
    // 1 kcal = 4.187 kJ
    // 1 kJ = 0.2388 kcal
    //

    // metric bmr formula [Tanita blog source]:
    // men = 66 + 13.7*weight(kg) + 5*height(cm) - 6.85*age(yr) [calories]
    // women = 655 + 9.6*weight(kg) + 1.85*height(cm) - 4.7*age(yr) [calories]
    //
    double wcoeff = ("male" == gender) ? 13.7f : 9.6f;
    double hcoeff = ("male" == gender) ? 5.0f : 1.85f;
    double acoeff = ("male" == gender) ? 6.85f : 4.7f;
    double bcoeff = ("male" == gender) ? 66.0f : 655.0f;

    if("imperial" == sys)
    {
       weight *= 0.453592f;
       height *= 2.54f;
    }
    double bmr = bcoeff + wcoeff*weight + hcoeff*height + acoeff*age;
    bmr = bmr * 4.184f / 1000.0f;
    m.setAttribute("basal_metabolic_rate", bmr, "kJ");

    addMeasurement(m);
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
      m.setAttribute("impedance", readImpedance(), "ohm");
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

QVariant BodyCompositionTest::readImpedance() const
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
