#include "HearingTest.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringBuilder>

HearingTest::HearingTest()
{
  m_outputKeyList << "patient_id";
  m_outputKeyList << "test_datetime";
  m_outputKeyList << "last_calibration_date";
  m_outputKeyList << "test_id";
}

bool HearingTest::isValid() const
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
    bool okTest = 16 == getNumberOfMeasurements();
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

bool HearingTest::isPartial() const
{
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
    return okTest;
}

QString HearingTest::toString() const
{
    QString str;
    if(isValid())
    {
        QStringList list;
        list << QString("patient id: ") % getMetaData("patient_id").toString();
        list << QString("test id: ") % getMetaData("test_id").toString();
        list << QString("test datetime: ") % getMetaData("test_datetime").toDateTime().toString("yyyy-MM-dd hh:mm:ss");
        list << QString("last calibration date: ") % getMetaData("last_calibration_date").toDate().toString("yyyy-MM-dd");
        foreach(auto m, m_measurementList)
        {
          list << m.toString();
        }
        str = list.join("\n");
    }
    return str;
}

QString HearingTest::readArray(const quint8 &begin, const quint8 &end) const
{
    int len = end - begin + 1;
    return (0 < len && end < m_array.size() ?
            QString::fromLatin1(m_array.mid(begin, len)) :
            QString());
}

// The AudiometerManager class provides the data after validating
// it via hasEndCode(arr) before passing to this class
//
void HearingTest::fromArray(const QByteArray &arr)
{
    if(!arr.isEmpty())
    {
        reset();
        m_array = arr;

        addMetaData("patient_id",readPatientID());
        addMetaData("test_id",readTestID());
        addMetaData("test_datetime",readTestDateTime());
        addMetaData("last_calibration_date",readCalibrationDate());

        QStringList sides = {"left","right"};
        foreach(auto side, sides)
        {
          foreach(auto m, readHearingThresholdLevels(side))
          {
            addMeasurement(m);
          }
        }
    }
}

QString HearingTest::readPatientID() const
{
    return readArray(4,17).trimmed();
}

QString HearingTest::readTestID() const
{
    return readArray(18,34).trimmed();
}

QDateTime HearingTest::readTestDateTime() const
{
    QString d_str = readArray(35,50).trimmed();
    if(d_str.contains("A"))
        d_str.replace("A","1");
    return QDateTime::fromString(d_str,"MM/dd/yyHH:mm:ss").addYears(100);
}

QDate HearingTest::readCalibrationDate() const
{
    QString d_str = readArray(51,58).trimmed();
    return QDate::fromString(d_str,"MM/dd/yy").addYears(100);
}

QString HearingTest::readExaminerID() const
{
    return readArray(59,74).trimmed();
}

QList<HearingMeasurement> HearingTest::readHearingThresholdLevels(const QString& side) const
{
  QList<HearingMeasurement> htl;
  QString str = ("left" == side.toLower()) ?
    readArray(75,106).trimmed() :
    readArray(107,138).trimmed();

  QStringList list = str.split(QRegExp("\\s+")).replaceInStrings(QRegExp("^\\s+|\\s+$"),"");
  int index = 0;
  foreach(auto code, list)
  {
    HearingMeasurement m(side, index++, code);
    htl.append(m);
  }

  return htl;
}

HearingMeasurement HearingTest::getMeasurement(const QString& side, const int& index) const
{
    HearingMeasurement measure;
    foreach(auto m, m_measurementList)
    {
        if(side == m.getAttributeValue("side").toString() &&
           HearingMeasurement::frequencyLookup[index] == m.getAttributeValue("test").toString())
        {
          measure = m;
          break;
        }
    }
    return measure;
}

QJsonObject HearingTest::toJsonObject() const
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
