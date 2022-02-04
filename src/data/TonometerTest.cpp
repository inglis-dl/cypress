#include "TonometerTest.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

TonometerTest::TonometerTest()
{
    // exam meta data
    m_outputKeyList << "measure_number"; // integer
    m_outputKeyList << "session_date";   // datetime
    m_outputKeyList << "patient_id";     //  string
    m_outputKeyList << "ora_serial_number"; // string (unique to site)
    m_outputKeyList << "ora_software";      // string
    m_outputKeyList << "pc_software";       // string
    m_outputKeyList << "meds";            // string (not applicable? never used)
    m_outputKeyList << "conditionss";     // string (not applicable? never used)
    m_outputKeyList << "notes_1";         // string (not applicable? never used)
    m_outputKeyList << "notes_2";         // string (not applicable? never used)
    m_outputKeyList << "notes_3";         // string (not applicable? never used)
    m_outputKeyList << "m_g2";            // double (not applicable? always 6.711 for both eyes)
    m_outputKeyList << "b_g2";            // double (not applicable? always 68)
    m_outputKeyList << "m_g3";            // double (not applicable? always 4.444)
    m_outputKeyList << "b_g3";            // double (not applicable? always -22.9)
    m_outputKeyList << "iop_cc_coef";     // double (not applicable? always 0.43)
    m_outputKeyList << "crf_coef";        // double (not applicable? always 0.7)
    m_outputKeyList << "m_abc";           // double (range 1.03 - 1.09)
    m_outputKeyList << "b_abc";           // double (range -23.85 - -3.42)
    m_outputKeyList << "b_pp";            // double (not applicable? always 6.12)
    m_outputKeyList << "best_weighted";   // integer/boolean (not applicable? always 0 or false)

    // data per eye
    m_outputKeyList << "measure_id";      // integer (range 1 - 3)
    m_outputKeyList << "measure_date";    // datetime
    m_outputKeyList << "eye";             // string (L,R)
    m_outputKeyList << "iopg";            // double (range 5.6 - 28.3)
    m_outputKeyList << "iopcc";           // double (range 0.8 - 34.8)
    m_outputKeyList << "crf";             // double (range 0.45 - 18.3)
    m_outputKeyList << "cct_avg";         // double (not applicable? always 0)
    m_outputKeyList << "cct_lowest";      // double (not applicable? always 0)
    m_outputKeyList << "cctsd";           // double (not applicable? always 0)
    m_outputKeyList << "ch";              // double (range 4.3 - 17.5)
    m_outputKeyList << "tear_film_value"; // double (range -14 - 31.4)
    m_outputKeyList << "pressure";        // comma delim string integer vector
    m_outputKeyList << "applanation";     // string integer vector
    m_outputKeyList << "time_in";         // double (range 5.7 - 9.8)
    m_outputKeyList << "time_out";        // double (16.4 - 21.3)
    m_outputKeyList << "quality_index";   // double (range 0.9 - 9.7)
    m_outputKeyList << "indexes";         // comma delim string double vector
}

void TonometerTest::fromQuery(QSqlQuery *query)
{
    if(!(query->isActive() && query->isSelect()))
    {
        qDebug() << "ERROR: cannot process query" << query->lastQuery();
        return;
    }
    QSqlRecord r = query->record();
    qDebug() << "first record" << QString::number(r.count()) << "fields";

    QVector<QVariant> head;
    QVector<QVariant::Type> field_type;
    for(int i=0;i<r.count();i++)
    {
      QSqlField f = r.field(i);
      qDebug() << f.name() << QMetaType::typeName(f.type());
      field_type.push_back(f.type());
      head << f.name();
    }

    /*
    if(QFileInfo::exists(fileName))
    {

    }
    */
}

// String representation for debug and GUI display purposes
//
QString TonometerTest::toString() const
{
    QString outStr;
    if (isValid())
    {
        QStringList tempList;
        for (auto&& measurement : m_measurementList)
        {
            tempList << measurement.toString();
        }
        outStr = tempList.join("\n");
    }
    return outStr;
}

bool TonometerTest::isValid() const
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
    bool okTest = 4 == getNumberOfMeasurements();
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

QJsonObject TonometerTest::toJsonObject() const
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
