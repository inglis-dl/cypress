#include "AudiometerTest.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringBuilder>

bool AudiometerTest::isValid() const
{
    bool okMeta =
      hasMetaDataCharacteristic("patient ID") &&
      hasMetaDataCharacteristic("test datetime") &&
      hasMetaDataCharacteristic("last calibration date") &&
      hasMetaDataCharacteristic("test ID");

    bool okTest = 16 == getNumberOfMeasurements();
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

bool AudiometerTest::isPartial() const
{
    bool okTest = 0 < getNumberOfMeasurements();
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

QString AudiometerTest::toString() const
{
    QString s;
    if(isValid())
    {
        QStringList l;
        l << QString("patient ID: ") % getMetaDataCharacteristic("patient ID").toString();
        l << QString("test ID: ") % getMetaDataCharacteristic("test ID").toString();
        l << QString("test datetime: ") % getMetaDataCharacteristic("test datetime").toDateTime().toString("yyyy-MM-dd hh:mm:ss");
        l << QString("last calibration date: ") % getMetaDataCharacteristic("last calibration date").toDate().toString("yyyy-MM-dd");
        for(auto&& x : m_measurementList)
        {
            l << x.toString();
        }
        s = l.join("\n");
    }
    return s;
}

QString AudiometerTest::readArray(const quint8 &begin, const quint8 &end) const
{
    int len = end-begin+1;
    return 0<len && end<m_array.size() ? QString::fromLatin1(m_array.mid(begin, len)) : QString();
}

// The AudiometerManager class provides the data after validating
// it via hasEndCode(arr) before passing to this class
//
void AudiometerTest::fromArray(const QByteArray &arr)
{
    if(!arr.isEmpty())
    {
        reset();
        m_array = arr;

        addMetaDataCharacteristic("patient ID",readPatientID());
        addMetaDataCharacteristic("test ID",readTestID());
        addMetaDataCharacteristic("test datetime",readTestDateTime());
        addMetaDataCharacteristic("last calibration date",readCalibrationDate());

        QList<HearingMeasurement> l_htl = readHearingThresholdLevels("left");
        QList<HearingMeasurement> r_htl = readHearingThresholdLevels("right");

        for(auto&& x : l_htl)
        {
          addMeasurement(x);
        }
        for(auto&& x : r_htl)
        {
          addMeasurement(x);
        }
    }
}

QString AudiometerTest::readPatientID() const
{
    return readArray(4,17).trimmed();
}

QString AudiometerTest::readTestID() const
{
    return readArray(18,34).trimmed();
}

QDateTime AudiometerTest::readTestDateTime() const
{
    QString d_str = readArray(35,50).trimmed();
    if(d_str.contains("A"))
        d_str.replace("A","1");
    return QDateTime::fromString(d_str,"MM/dd/yyHH:mm:ss").addYears(100);
}

QDate AudiometerTest::readCalibrationDate() const
{
    QString d_str = readArray(51,58).trimmed();
    return QDate::fromString(d_str,"MM/dd/yy").addYears(100);
}

QString AudiometerTest::readExaminerID() const
{
    return readArray(59,74).trimmed();
}

QList<HearingMeasurement> AudiometerTest::readHearingThresholdLevels(const QString& side) const
{
  QList<HearingMeasurement> htl;
  QString s = ("left" == side.toLower()) ?
    readArray(75,106).trimmed() :
    readArray(107,138).trimmed();

  QStringList s_list = s.split(QRegExp("\\s+")).replaceInStrings(QRegExp("^\\s+|\\s+$"),"");
  int i_freq = 0;
  foreach(auto atom, s_list)
  {
    HearingMeasurement m;
    m.fromCode(side, i_freq++, atom);
    htl.append(m);
  }

  return htl;
}

HearingMeasurement AudiometerTest::getMeasurement(const QString& side, const int& index) const
{
    HearingMeasurement m;
    for(auto&& x : m_measurementList)
    {
        if(side == x.getCharacteristic("side").toString() &&
           HearingMeasurement::frequencyLookup[index] == x.getCharacteristic("test"))
        {
            m = x;
            break;
        }
    }
    return m;
}

QJsonObject AudiometerTest::toJsonObject() const
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
