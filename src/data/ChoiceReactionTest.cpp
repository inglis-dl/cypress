#include "ChoiceReactionTest.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

// the minimum output data keys required from a successful a test
//
ChoiceReactionTest::ChoiceReactionTest()
{
    m_outputKeyList << "user_id";
    m_outputKeyList << "start_datetime";
    m_outputKeyList << "end_datetime";
    m_outputKeyList << "interviewer_id";
    m_outputKeyList << "number_of_measurements";
    m_outputKeyList << "version";
}

void ChoiceReactionTest::fromFile(const QString &fileName)
{
    QFile ifile(fileName);
    if(ifile.open(QIODevice::ReadOnly))
    {
        qDebug() << "OK, reading input file " << fileName;

        QTextStream stream(&ifile);
        int clinic_pos = -1;
        int version_pos = -1;
        int userid_pos = -1; // the participant's interview barcode
        int interviewerid_pos = -1;  // the interviewer id

        reset();

        qDebug() << "begin reading text stream";
        int n_line = 0;
        while(!stream.atEnd())
        {
            QString s = stream.readLine();
            QStringList l = s.split(",");
            qDebug() << "reading line "<< QString::number(++n_line) <<" with number of items = " << QString::number(l.size());
            if(!l.empty())
            {
                int code = l.at(0).toInt();
                if((ChoiceReactionMeasurement::TEST_CODE-1) == code)
                {
                    // get the position of the meta data key:
                    // Version, InterviewerId, UserId, Clinic
                    //
                    clinic_pos = l.indexOf(QLatin1String("Clinic"));
                    version_pos = l.indexOf(QLatin1String("Version"));
                    userid_pos = l.indexOf(QLatin1String("UserId"));
                    interviewerid_pos = l.indexOf(QLatin1String("InterviewerId"));

                    qDebug() << "found header item positions at line " << QString::number(n_line);
                }
                else if(ChoiceReactionMeasurement::TEST_CODE == code)
                {
                    ChoiceReactionMeasurement m;
                    m.fromString(s);                    
                    addMeasurement(m);
                    qDebug() << "found "<<(m.isValid()?"VALID":"INVALID")<<"measurement item positions at line " << QString::number(n_line);
                    qDebug() << m.toString();
                }
                else if((ChoiceReactionMeasurement::TEST_CODE+1) == code)
                {
                    qDebug() << "found last line " << QString::number(n_line);

                    if(-1 != version_pos)
                    {
                      addMetaDataCharacteristic("version",l.at(version_pos));
                      qDebug() << "adding version meta info";
                    }
                    if(-1 != clinic_pos)
                    {
                      addMetaDataCharacteristic("clinic",l.at(clinic_pos));
                      qDebug() << "adding clinic meta info";
                    }
                    if(-1 != userid_pos)
                    {
                      addMetaDataCharacteristic("user_id",l.at(userid_pos));
                      qDebug() << "adding user id meta info";
                    }
                    if(-1 != interviewerid_pos)
                    {
                      addMetaDataCharacteristic("interviewer_id",l.at(interviewerid_pos));
                      qDebug() << "adding interviewer id meta info";
                    }
                    // get the position of the following meta data keys
                    // that are within the last line of the file with the correct code:
                    // EndDateTime, StartDateTimes
                    //
                    int pos = l.indexOf(QLatin1String("EndDateTime"));
                    if(-1 != pos)
                    {
                        QString s = l.at(pos+1);
                        s = s.remove(QRegExp("\\s(AM|PM)$")).trimmed();
                        qDebug() << "end datetime string " << s;
                        QDateTime d = QDateTime::fromString(s, "MM/dd/yyyy HH:mm:ss");
                        addMetaDataCharacteristic("end_datetime",d);
                        qDebug() << "adding end datetime meta info";
                    }
                    pos = l.indexOf(QLatin1String("StartDateTimes"));
                    if(-1 != pos)
                    {
                        QString s = l.at(pos+1);
                        s = s.remove(QRegExp("\\s(AM|PM)$")).trimmed();
                        qDebug() << "start datetime string " << s;

                        QDateTime d = QDateTime::fromString(s, "MM/dd/yyyy HH:mm:ss");
                        addMetaDataCharacteristic("start_datetime",d);
                        qDebug() << "adding start datetime meta info";
                    }
                }
                else
                {
                    qDebug() << "ERROR: cannot parse line, bad code";
                }
            }
            // additional meta data: the number of measurements
            int n = getNumberOfMeasurements();
            addMetaDataCharacteristic("number_of_measurements",n);
        }
        qDebug() << "closed stream";
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
    bool okMeta = true;
    for(auto&& x : m_outputKeyList)
    {
      if(!hasMetaDataCharacteristic(x))
      {
         okMeta = false;
         qDebug() << "ERROR: missing test meta data " << x;
         break;
       }
    }
    bool okTest = 0 < getNumberOfMeasurements();
    if(okTest)
    {
      for(auto&& x : m_measurementList)
      {
        if(!x.isValid())
        {
          okTest = false;
          qDebug() << "ERROR: invalid test measurement";
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
        jsonArr.append(x.toJsonObject());
    }
    QJsonObject json;
    json.insert("test_meta_data",m_metaData.toJsonObject());
    json.insert("test_results",jsonArr);
    return json;
}
