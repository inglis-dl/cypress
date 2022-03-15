#include "TonometerTest.h"
#include "../auxiliary/Utilities.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <algorithm>

QMap<QString,QString> TonometerTest::metaLUT = TonometerTest::initMetaLUT();

QMap<QString,QString> TonometerTest::initMetaLUT()
{
  QMap<QString,QString> map;

  map["id"] = "ID";
  map["date_of_birth"] = "BirthDate";
  map["sex"] = "Sex";
  map["measure_number"] = "MeasureNumber";
  map["session_datetime"] = "SessionDate";
  map["patient_id"] = "PatientID";
  map["ora_serial_number"] = "ORASerialNumber";
  map["ora_software"] = "ORASoftware";
  map["pc_software"] = "PCSoftware";
  map["meds"] = "Meds";
  map["conditions"] = "Conditions";
  map["notes_1"] = "Notes1";
  map["notes_2"] = "Notes2";
  map["notes_3"] = "Notes3";
  map["m_g2"] = "m_G2";
  map["b_g2"] = "b_G2";
  map["m_g3"] = "m_G3";
  map["b_g3"] = "b_G3";
  map["iop_cc_coef"] = "iop_cc_coef";
  map["crf_coef"] = "crf_coef";
  map["m_abc"] = "m_ABC";
  map["b_abc"] = "b_ABC";
  map["b_pp"] = "b_PP";
  map["best_weighted"] = "BestWeighted";

  return map;
}

TonometerTest::TonometerTest()
{
    // exam meta data
    m_outputKeyList << "id"; // in "ID" from Patients table
    m_outputKeyList << "date_of_birth"; // in "BirthDate" from Patients table
    m_outputKeyList << "sex"; // in "Sex" from Patients table

    m_outputKeyList << "measure_number"; // int "MeasureNumber"
    m_outputKeyList << "session_datetime";   // datetime "SessionDate"
    m_outputKeyList << "patient_id";     //  int "PatientID" primary key in Patients table
    m_outputKeyList << "ora_serial_number"; // string (unique to site) "ORASerialNmber"
    m_outputKeyList << "ora_software";      // string "ORASoftware"
    m_outputKeyList << "pc_software";       // string "PCSoftware"
    m_outputKeyList << "meds";            // string (not applicable? never used) "Meds"
    m_outputKeyList << "conditions";      // string (not applicable? never used) "Conditions"
    m_outputKeyList << "notes_1";         // string (not applicable? never used) "Notes1"
    m_outputKeyList << "notes_2";         // string (not applicable? never used) "Notes2"
    m_outputKeyList << "notes_3";         // string (not applicable? never used) "Notes3"
    m_outputKeyList << "m_g2";            // double (not applicable? always 6.711 for both eyes) "m_G2"
    m_outputKeyList << "b_g2";            // double (not applicable? always 68) "b_G2"
    m_outputKeyList << "m_g3";            // double (not applicable? always 4.444) "m_G3"
    m_outputKeyList << "b_g3";            // double (not applicable? always -22.9) "b_G3"
    m_outputKeyList << "iop_cc_coef";     // double (not applicable? always 0.43) "iop_cc_coef"
    m_outputKeyList << "crf_coef";        // double (not applicable? always 0.7) "crf_coef"
    m_outputKeyList << "m_abc";           // double (range 1.03 - 1.09) "m_ABC"
    m_outputKeyList << "b_abc";           // double (range -23.85 - -3.42) "b_ABC"
    m_outputKeyList << "b_pp";            // double (not applicable? always 6.12) "b_PP"
    m_outputKeyList << "best_weighted";   // uint (not applicable? always 0 or false) "BestWeighted"
}

void TonometerTest::fromJson(const QJsonArray &json)
{
    // expecting an array of json objects,
    // one for each row of the ORA mdb Measures table
    //
    if(0 == json.size()) return;

    // check if there are multiple session dates and retain
    // data from the most recent
    //
    QList<QDateTime> dateList;
    foreach(auto x, json)
    {
        QJsonObject obj = x.toObject();
        if(obj.contains("SessionDate"))
        {
          QDateTime t = obj["SessionDate"].toVariant().toDateTime();
          if(!dateList.contains(t))
            dateList.push_back(t);
        }
    }

    std::sort(dateList.begin(),dateList.end());
    foreach(auto x, dateList)
    {
      qDebug() << "sorted session" << x.toString();
    }
    QDateTime lastSession = dateList.last();

    bool meta = false;
    foreach(auto x, json)
    {
        QJsonObject obj = x.toObject();
        if(obj.contains("SessionDate") && obj.contains("Eye"))
        {
          QDateTime t = obj["SessionDate"].toVariant().toDateTime();
          if(lastSession == t)
          {
            // all sessions on one date share the same meta data
            //
            if(!meta)
            {
               QMap<QString,QString>::const_iterator it = TonometerTest::metaLUT.constBegin();
               while(it != TonometerTest::metaLUT.constEnd())
               {
                  if(obj.contains(it.value()))
                  {
                    addMetaData(it.key(),obj[it.value()].toVariant());
                  }
                  it++;
               }
               meta = true;
            }

            TonometerMeasurement m;
            m.fromJson(obj);
            if(m.isValid())
            {
              addMeasurement(m);
            }
          }
        }
    }
}

QStringList TonometerTest::getMeasurementStrings(const QString &side) const
{
    QStringList list;
    if(isValid())
    {
      foreach(auto m, m_measurementList)
      {
        if(side == m.getAttribute("side").toString())
        {
            QMap<QString,QString>::const_iterator it = TonometerMeasurement::variableLUT.constBegin();
            while(it != TonometerMeasurement::variableLUT.constEnd())
            {
               QString key = it.key();
               it++;
               if("side" == key) continue;
               list << QString("%1: %2").arg(key,m.getAttribute(key).toString());
            }
        }
      }
    }
    return list;
}

void TonometerTest::simulate(const QMap<QString, QVariant> &input)
{
    reset();
    qDebug() << "generating simulated data";
    addMetaData("id",input["barcode"]);
    QVariant value = input["sex"].toString().toLower().startsWith("f") ? 0 : -1;
    addMetaData("sex",value);
    addMetaData("date_of_birth",input["date_of_birth"].toDateTime());

    double mu = QRandomGenerator::global()->generateDouble();

    addMetaData("measure_number",1);
    addMetaData("session_datetime",QDateTime::currentDateTime());
    addMetaData("patient_id",QRandomGenerator::global()->bounded(1000, 9999));
    addMetaData("ora_serial_number","000073158");
    addMetaData("ora_software","2.11");
    addMetaData("pc_software","3.01");
    addMetaData("meds",QString(""));
    addMetaData("conditions",QString(""));
    addMetaData("notes_1",QString(""));
    addMetaData("notes_2",QString(""));
    addMetaData("notes_3",QString(""));
    addMetaData("m_g2",6.711f);
    addMetaData("b_g2",68.0f);
    addMetaData("m_g3",4.444f);
    addMetaData("b_g3",-22.9f);
    addMetaData("iop_cc_coef",0.43f);
    addMetaData("crf_coef",0.7f);
    addMetaData("m_abc",Utilities::interp(1.03,1.09,mu));
    addMetaData("b_abc",Utilities::interp(-23.85,-3.42,mu));
    addMetaData("b_pp",6.12f);
    addMetaData("best_weighted",0);

    QStringList sides = {"left","right"};
    foreach(auto side, sides)
    {
        TonometerMeasurement m;
        m.simulate(side);
        if(m.isValid())
        {
          addMeasurement(m);
        }
        else
          qDebug() << "ERROR: simulated measurement is invalid";
    }
}

// String representation for debug purposes
//
QString TonometerTest::toString() const
{
    QString str;
    if(isValid())
    {
      QStringList list;
      foreach(auto m, m_measurementList)
      {
         list << m.toString();
      }
      str = list.join("\n");
    }
    return str;
}

bool TonometerTest::isValid() const
{
    bool okMeta = true;
    foreach(auto key, m_outputKeyList)
    {
      if(!hasMetaData(key))
      {
         qDebug() << "error, test is missing metadata" << key;
         okMeta = false;
         break;
       }
    }
    bool okTest = 2 == getNumberOfMeasurements();
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

QJsonObject TonometerTest::toJsonObject() const
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
