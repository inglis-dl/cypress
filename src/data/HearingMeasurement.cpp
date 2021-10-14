#include "HearingMeasurement.h"

#include <QDateTime>
#include <QDebug>

void HearingMeasurement::fromArray(const QByteArray &arr)
{
    if(!arr.isEmpty() && hasEndCode(arr))
    {
        m_array = arr;



      // each measurement atom is a dB response for a specific frequency
        /*
      QByteArray bytes(arr.simplified());
      QList<QByteArray> parts = bytes.split(' ');
      if(3 <= parts.size())
      {
        m_characteristicValues["weight"] = QString::number(parts[0].toFloat(),'f',1);
        m_characteristicValues["units"] = QString(parts[1]);
        m_characteristicValues["mode"] = QString(parts[2]);
        m_characteristicValues["timestamp"] = QDateTime::currentDateTime();
       */
 /*
  *    opal expects
  *
RES_TEST_ID text
RES_TEST_DATETIM datetime
RES_LAST_CALIBRATION_DATE datetime
RES_RIGHT_1KT integer
RES_RIGHT_500 integer
RES_RIGHT_1K integer
RES_RIGHT_2K integer
RES_RIGHT_3K integer
RES_RIGHT_4K integer
RES_RIGHT_6K integer
RES_RIGHT_8K integer
RES_LEFT_1KT integer
RES_LEFT_500 integer
RES_LEFT_1K integer
RES_LEFT_2K integer
RES_LEFT_3K integer
RES_LEFT_4K integer
RES_LEFT_6K integer
RES_LEFT_8K integer
RES_RIGHT_1KT_ERR tex
RES_RIGHT_500_ERR text
RES_RIGHT_1K_ERR text
RES_RIGHT_2K_ERR text
RES_RIGHT_3K_ERR text
RES_RIGHT_4K_ERR text
RES_RIGHT_6K_ERR text
RES_RIGHT_8K_ERR text
RES_LEFT_1KT_ERR text
RES_LEFT_500_ERR text
RES_LEFT_1K_ERR text
RES_LEFT_2K_ERR text
RES_LEFT_3K_ERR text
RES_LEFT_4K_ERR text
RES_LEFT_6K_ERR text
RES_LEFT_8K_ERR
  *
  *
  */



    }
}

QString HearingMeasurement::readArray(const quint8 &begin, const quint8 &end) const
{
    int len = end-begin+1;
    qDebug() << "reading bytes from " << begin << " to " << end;
    return 0<len && end<m_array.size() ? QString::fromLatin1(m_array.mid(begin, len)) : QString();
}

char HearingMeasurement::getFlag() const
{
    return m_array.at(0);
}

QString HearingMeasurement::getPatientID() const
{
    return readArray(4,17).trimmed();
}

QString HearingMeasurement::getTestID() const
{
    return readArray(18,34).trimmed();
}

QDateTime HearingMeasurement::getTestDateTime() const
{
    QString d_str = readArray(35,50).trimmed();
    if(d_str.contains("A"))
        d_str.replace("A","1");
    qDebug() << "test date string " << d_str;
    return QDateTime::fromString(d_str,"MM/dd/yyHH:mm:ss").addYears(100);
}

QDate HearingMeasurement::getCalibrationDate() const
{
    QString d_str = readArray(51,58).trimmed();
    qDebug() << "calib date string " << d_str;
    return QDate::fromString(d_str,"MM/dd/yy").addYears(100);
}

QString HearingMeasurement::getExaminerID() const
{
    return readArray(59,74).trimmed();
}

QMap<QString,quint8> HearingMeasurement::getLeftHearingTestLevels() const
{
    QString s = readArray(75,106).trimmed();
    qDebug() << "HTL left: " << s;
    QStringList s_list = s.split(QRegExp("\\s+")).replaceInStrings(QRegExp("^\\s+|\\s+$"),"");
    int i=0;
    foreach(QString atom , s_list)
      qDebug() << "htl left " << QString::number(i++) << " " << atom;

    QMap<QString,quint8> htl;
    //htl["1KT"] =
    return htl;
}

QMap<QString,quint8> HearingMeasurement::getRightHearingTestLevels() const
{
    QString s = readArray(107,138).trimmed();
    qDebug() << "HTL right: " << s;
    QStringList s_list = s.split(QRegExp("\\s+")).replaceInStrings(QRegExp("^\\s+|\\s+$"),"");
    int i=0;
    foreach(QString atom , s_list)
      qDebug() << "htl right " << QString::number(i++) << " " << atom;

    QMap<QString,quint8> htl;
    //htl["1KT"] =
    return htl;

}

QMap<QString,QString> HearingMeasurement::getLeftHearingTestCodes() const
{
    QMap<QString,QString> htl;
    //htl["1KT"] =
    return htl;

}

QMap<QString,QString> HearingMeasurement::getRightHearingTestCodes() const
{
    QMap<QString,QString> htl;
    //htl["1KT"] =
    return htl;

}

QMap<QString,QString> HearingMeasurement::getLeftHearingTestOutcomes() const
{
    QMap<QString,QString> htl;
    //htl["1KT"] =
    return htl;

}

QMap<QString,QString> HearingMeasurement::getRightHearingTestOutcomes() const
{
    QMap<QString,QString> htl;
    //htl["1KT"] =
    return htl;

}

bool HearingMeasurement::isValid() const
{
    bool ok = true;
    if(m_array.isEmpty() || !hasEndCode(m_array)) return false;
    return ok;
}

QString HearingMeasurement::toString() const
{
  return QString();
}

bool HearingMeasurement::hasEndCode(const QByteArray &arr)
{
    if( arr.size() < 6 ) return false;
    // try and interpret the last 6 bytes
    int size = arr.size();
    return (
       0x0d == arr.at(size-1) &&
       0x17 == arr.at(size-4) &&
        'p' == arr.at(size-5) &&
        '~' == arr.at(size-6));
}
