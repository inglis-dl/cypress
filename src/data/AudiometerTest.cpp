#include "AudiometerTest.h"

#include <QDateTime>
#include <QDebug>

AudiometerTest::AudiometerTest(const AudiometerTest &other) : TestBase(other)
{
   m_array = other.m_array;
}

AudiometerTest& AudiometerTest::operator=(const AudiometerTest &other)
{
    TestBase::operator=(other);
    m_array = other.m_array;
    return *this;
}

bool AudiometerTest::isValid() const
{
    bool ok = true;
    return ok;
}

QString AudiometerTest::toString() const
{
    qDebug() << m_metadata;
    foreach(auto x, m_measurementList)
        qDebug() << x;
    return QString();
}

void AudiometerTest::reset()
{
    TestBase::reset();
    m_array.clear();
}

QString AudiometerTest::readArray(const quint8 &begin, const quint8 &end) const
{
    int len = end-begin+1;
    qDebug() << "reading bytes from " << begin << " to " << end;
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

        addMetadata("patient ID",getPatientID());
        addMetadata("test ID",getTestID());
        addMetadata("test datetime",getTestDateTime());
        addMetadata("last calibration date",getCalibrationDate());

        QList<HearingMeasurement> l_htl = getHearingThresholdLevels("left");
        QList<HearingMeasurement> r_htl = getHearingThresholdLevels("right");

        foreach(auto x, l_htl)
        {
            addMeasurement(x);
        }
        foreach(auto x, r_htl)
        {
            addMeasurement(x);
        }
    }
}

QString AudiometerTest::getPatientID() const
{
    return readArray(4,17).trimmed();
}

QString AudiometerTest::getTestID() const
{
    return readArray(18,34).trimmed();
}

QDateTime AudiometerTest::getTestDateTime() const
{
    QString d_str = readArray(35,50).trimmed();
    if(d_str.contains("A"))
        d_str.replace("A","1");
    qDebug() << "test date string " << d_str;
    return QDateTime::fromString(d_str,"MM/dd/yyHH:mm:ss").addYears(100);
}

QDate AudiometerTest::getCalibrationDate() const
{
    QString d_str = readArray(51,58).trimmed();
    qDebug() << "calib date string " << d_str;
    return QDate::fromString(d_str,"MM/dd/yy").addYears(100);
}

QString AudiometerTest::getExaminerID() const
{
    return readArray(59,74).trimmed();
}

QList<HearingMeasurement> AudiometerTest::getHearingThresholdLevels(const QString& side) const
{
  QList<HearingMeasurement> htl;
  QString s = ("left" == side.toLower()) ?
    readArray(75,106).trimmed() :
    readArray(107,138).trimmed();

  qDebug() << "HTL " << side;
  QStringList s_list = s.split(QRegExp("\\s+")).replaceInStrings(QRegExp("^\\s+|\\s+$"),"");
  int i=0;
  foreach(QString atom , s_list)
  {
    HearingMeasurement m;
    m.fromCode(i,atom);
    m.setCharacteristic("side",side.toLower());
    htl.append(m);
    qDebug() << "htl value " << QString::number(i++) << " " << atom;
  }

  return htl;
}
