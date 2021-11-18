#include "ChoiceReactionTest.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

void ChoiceReactionTest::fromFile(const QString &fileName)
{
    QFile ifile(fileName);
    if(ifile.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&ifile);
        int version_pos = -1;
        int userid_pos = -1; // the interviewer's id
        int interviewid_pos = -1;  // should be the same as the barcode
        reset();

        while(!stream.atEnd())
        {
            QString s = stream.readLine();
            QStringList l = s.split(",");
            if(0<l.size())
            {
                int code = l.at(0).toInt();
                if((ChoiceReactionMeasurement::TEST_CODE-1) == code)
                {
                    // get the position of the meta data key:
                    // Version, InterviewerId, UserId
                    //
                    version_pos = l.indexOf(QLatin1String("Version"));
                    userid_pos = l.indexOf(QLatin1String("UserId"));
                    interviewid_pos = l.indexOf(QLatin1String("InterviewerId"));
                }
                else if(ChoiceReactionMeasurement::TEST_CODE == code)
                {
                    ChoiceReactionMeasurement m;
                    m.fromString(s);
                    addMeasurement(m);
                }
                else if((ChoiceReactionMeasurement::TEST_CODE+1) == code)
                {
                    if(-1 != version_pos)
                    {
                      addMetaDataCharacteristic("version",l.at(version_pos));
                    }
                    if(-1 != userid_pos)
                    {
                      addMetaDataCharacteristic("user id",l.at(userid_pos));
                    }
                    if(-1 != interviewid_pos)
                    {
                      addMetaDataCharacteristic("interview id",l.at(interviewid_pos));
                    }
                    // get the position of the following meta data keys
                    // that are within the last line of the file with the correct code:
                    // EndDateTime, StartDateTimes
                    //
                    int pos = l.indexOf(QLatin1String("EndDateTime"))+1;
                    if(0 != pos)
                    {
                        QString s = l.at(pos+1);
                        s = s.remove(QRegExp("\\s(AM|PM)$")).trimmed();
                        QDateTime d = QDateTime::fromString(s, "MM/dd/YYYY HH:mm:ss");
                        addMetaDataCharacteristic("end datetime",d);
                    }
                    pos = l.indexOf(QLatin1String("StartDateTimes"))+1;
                    if(0 != pos)
                    {
                        QString s = l.at(pos+1);
                        s = s.remove(QRegExp("\\s(AM|PM)$")).trimmed();
                        QDateTime d = QDateTime::fromString(s, "MM/dd/YYYY HH:mm:ss");
                        addMetaDataCharacteristic("start datetime",d);
                    }
                }
                else
                {
                    qDebug() << "ERROR: cannot parse line, bad code";
                }
            }
            // additional meta data: the number of measurements
            int n = getNumberOfMeasurements();
            addMetaDataCharacteristic("number of measurements",n);
        }
        ifile.close();
    }
}

// String representation for debug and GUI display purposes
//
QString ChoiceReactionTest::toString() const
{
    QString s;
    if(isValid())
    {
        QStringList l;
        for(auto&& x : m_measurementList)
        {
            l << x.toString();
        }
        s = l.join("\n");
    }
    return s;
}

bool ChoiceReactionTest::isValid() const
{
    bool okMeta =
      hasMetaDataCharacteristic("user id") &&
      hasMetaDataCharacteristic("start datetime") &&
      hasMetaDataCharacteristic("end datetime") &&
      hasMetaDataCharacteristic("interview id") &&
      hasMetaDataCharacteristic("number of measurements") &&
      hasMetaDataCharacteristic("version");

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
    return okMeta && okTest;
}

// String keys are converted to snake_case
//
QJsonObject ChoiceReactionTest::toJsonObject() const
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
