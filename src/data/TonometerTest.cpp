#include "TonometerTest.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>

QMap<QString,QString> TonometerTest::variableLUT = TonometerTest::initVariableLUT();
QMap<QString,QString> TonometerTest::metaLUT = TonometerTest::initMetaLUT();
QMap<QString,QString> TonometerTest::unitsLUT = TonometerTest::initUnitsLUT();

QMap<QString,QString> TonometerTest::initVariableLUT()
{
  QMap<QString,QString> map;

  map["measure_id"] = "MeasureID";
  map["measure_datetime"] = "MeasureDate";
  map["side"] = "Eye";
  map["iopg"] = "IOPG";
  map["iopcc"] = "IOPCC";
  map["crf"] = "CRF";
  map["cct_avg"] = "CCTAvg";
  map["cct_lowest"] = "CCTLowest";
  map["cct_sd"] = "CCTSD";
  map["ch"] = "CH";
  map["tear_film_value"] = "TearFilmValue";
  map["pressure"] = "Pressure";
  map["applanation"] = "Applanation";
  map["time_in"] = "TimeIn";
  map["time_out"] = "TimeOut";
  map["quality_index"] = "QualityIndex";
  map["indexes"] = "Indexes";

  return map;
}

QMap<QString,QString> TonometerTest::initMetaLUT()
{
  QMap<QString,QString> map;

  map["measure_number"] = "MeasureNumber";
  map["session_datetime"] = "SessionDate";
  map["patient_id"] = "PatientID";
  map["ora_serial_number"] = "ORASerialNmber";
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

QMap<QString,QString> TonometerTest::initUnitsLUT()
{
  QMap<QString,QString> map;

  map["iopg"] = "mmHg";
  map["iopcc"] = "mmHg";
  map["crf"] = "mmHg";
  map["ch"] = "mmHg";
  map["cct_avg"] = "um";
  map["cct_avg"] = "um";
  map["cct_avg"] = "um";
  map["time_in"] = "ms";
  map["time_out"] = "ms";

  return map;
}

TonometerTest::TonometerTest()
{
    // exam meta data
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

    // data per eye
    m_outputKeyList << "measure_id";      // int (range 1 - 3) "MeasureID" primary key in Measures table
    m_outputKeyList << "measure_datetime";    // datetime "MeasureDate"
    m_outputKeyList << "side";            // string (L,R) "Eye" (convert to {left,right}
    m_outputKeyList << "iopg";            // double (range 5.6 - 28.3) mmHg "IOPG" : IOP = Goldmann-correlated intraocular pressure (IOP)
    m_outputKeyList << "iopcc";           // double (range 0.8 - 34.8) mmHg "IOPCC" : corneal-compensated IOP
    m_outputKeyList << "crf";             // double (range 0.45 - 18.3) mmHg "CRF" : corneal resistance factor
    m_outputKeyList << "cct_avg";         // double (not applicable? always 0) um "CCTAvg" : CCT = central corneal thickness
    m_outputKeyList << "cct_lowest";      // double (not applicable? always 0) um "CCTLowest"
    m_outputKeyList << "cct_sd";           // double (not applicable? always 0) um "CCTSD"
    m_outputKeyList << "ch";              // double (range 4.3 - 17.5) mmHg "CH" : corneal hysteresis
    m_outputKeyList << "tear_film_value"; // double (range -14 - 31.4) "TearFilmValue"
    m_outputKeyList << "pressure";        // string comma delim int vector "Pressure"
    m_outputKeyList << "applanation";     // string comma delim int vector "Applanation"
    m_outputKeyList << "time_in";         // double (range 5.7 - 9.8) msec "TimeIn"
    m_outputKeyList << "time_out";        // double (16.4 - 21.3) msec "TimeOut"
    m_outputKeyList << "quality_index";   // double (range 0 - 10) "QualityIndex" : waveform score
    m_outputKeyList << "indexes";         // string comma delim double vector "Indexes"
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
    for(auto&& x : json)
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
    for(auto&& x : dateList)
    {
      qDebug() << "sorted session" << x.toString();
    }
    QDateTime lastSession = dateList.last();

    bool meta = false;
    for(auto&& x : json)
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
                    addMetaDataCharacteristic(it.key(),obj[it.value()].toVariant());
                  ++it;
               }
               meta = true;
            }

            QString side = "L" == obj["Eye"].toVariant().toString() ? "left" : "right";
            QMap<QString,QString>::const_iterator it = TonometerTest::variableLUT.constBegin();
            while(it != TonometerTest::variableLUT.constEnd())
            {
               QString key = it.key();
               if("side" != key && obj.contains(it.value()))
               {
                 TonometerMeasurement m;
                 m.setCharacteristic("name", key);
                 m.setCharacteristic("value", obj[it.value()].toVariant());
                 m.setCharacteristic("side",side);
                 m.setCharacteristic("units",
                   TonometerTest::unitsLUT.contains(key) ? TonometerTest::unitsLUT[key] : QVariant());

                 addMeasurement(m);
               }
               ++it;
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
        for(auto&& x : m_measurementList)
        {
            if(side == x.getCharacteristic("side").toString())
            {
                list.push_back(x.toString());
            }
        }
    }
    return list;
}

// String representation for debug purposes
//
QString TonometerTest::toString() const
{
    QString outStr;
    if(isValid())
    {
        QStringList tempList;
        for(auto&& measurement : m_measurementList)
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
      if(!TonometerTest::metaLUT.contains(x)) continue;
      if(!hasMetaDataCharacteristic(x))
      {
         okMeta = false;
         qDebug() << "ERROR: missing test meta data " << x;
         break;
       }
    }
    bool okTest = TonometerTest::variableLUT.size() == getNumberOfMeasurements();
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
