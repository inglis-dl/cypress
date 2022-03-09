#include "TonometerMeasurement.h"

#include "../auxiliary/Utilities.h"
#include <QDebug>
#include <QJsonObject>
#include <QRandomGenerator>

/*
 *
 *    // data per eye
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
 *
*/
QMap<QString,QString> TonometerMeasurement::variableLUT = TonometerMeasurement::initVariableLUT();
QMap<QString,QString> TonometerMeasurement::unitsLUT = TonometerMeasurement::initUnitsLUT();

QMap<QString,QString> TonometerMeasurement::initVariableLUT()
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

QMap<QString,QString> TonometerMeasurement::initUnitsLUT()
{
  QMap<QString,QString> map;

  map["iopg"] = "mmHg";
  map["iopcc"] = "mmHg";
  map["crf"] = "mmHg";
  map["ch"] = "mmHg";
  map["pressure"] = "mmHg";
  map["applanation"] = "mmHg";
  map["cct_avg"] = "um";
  map["cct_lowest"] = "um";
  map["cct_sd"] = "um";
  map["time_in"] = "ms";
  map["time_out"] = "ms";

  return map;
}

bool TonometerMeasurement::isValid() const
{
    bool ok = true;
    QMap<QString,QString>::const_iterator it = TonometerMeasurement::variableLUT.constBegin();
    while(it != TonometerMeasurement::variableLUT.constEnd())
    {
      if(!hasAttribute(it.key()))
      {
        ok = false;
        break;
      }
      it++;
    }
    return ok;
}

void TonometerMeasurement::fromJson(const QJsonObject& obj)
{
    reset();
    QString side = "L" == obj["Eye"].toVariant().toString() ? "left" : "right";
    setAttribute("side",side);
    QMap<QString,QString>::const_iterator it = TonometerMeasurement::variableLUT.constBegin();
    while(it != TonometerMeasurement::variableLUT.constEnd())
    {
       QString key = it.key();
       if("side" != key && obj.contains(it.value()))
       {
         QString units = TonometerMeasurement::unitsLUT.contains(key) ? TonometerMeasurement::unitsLUT[key] : QString();
         setAttribute(key,Measurement::Value(obj[it.value()].toVariant(),units));
       }
       it++;
    }
}

void TonometerMeasurement::simulate(const QString& side)
{
    double mu = QRandomGenerator::global()->generateDouble();
    reset();
    setAttribute("side",side);
    qDebug() <<"echo"<< getAttribute("side").toString();

    QMap<QString,QString>::const_iterator it = TonometerMeasurement::variableLUT.constBegin();
    int sample = qRound(Utilities::interp(37.0f,74.0f,mu));
    while(it != TonometerMeasurement::variableLUT.constEnd())
    {
      QString key = it.key();
      it++;
      if("side" == key) continue;

      QVariant value;
      if(key.endsWith("datetime"))
          value = QDateTime::currentDateTime();
      else if("time_in" == key)
          value = Utilities::interp(5.7f,9.8f,mu);
      else if("time_out" == key)
          value = Utilities::interp(16.4f,21.3f,mu);
      else if(key.startsWith("cct_"))
          value = 0.0f;
      else if("pressure"==key || "applanation" == key || "indexes" == key)
      {
          QStringList list;
          if("indexes" == key)
          {
            for(int i=0;i<sample;i++)
            {
              mu = QRandomGenerator::global()->generateDouble();
              double v = Utilities::interp(0.2f,7725.0f,mu);
              list.append(QString::number(v,'f',3));
            }
          }
          else
          {
            int v_min = "pressure"==key ? 15 : 0;
            int v_max = "pressure"==key ? 525 : 920;
            for(int i=0;i<sample;i++)
            {
              mu = QRandomGenerator::global()->generateDouble();
              int v = qRound(Utilities::interp(v_min,v_max,mu));
              list.append(QString::number(v));
            }
          }
          value = list.join(",");
      }
      else if("iopg" == key)
          value = Utilities::interp(5.6f,28.3f,mu);
      else if("iopcc" == key)
          value = Utilities::interp(0.8f,34.8f,mu);
      else if("crf" == key)
          value = Utilities::interp(0.45f,18.3f,mu);
      else if("ch" == key)
          value = Utilities::interp(4.3f,17.5f,mu);
      else if("tear_film_value" == key)
          value = Utilities::interp(-14.0f,31.4f,mu);
      else if("quality_index" == key)
          value = Utilities::interp(0.0f,10.0f,mu);
      else
          value = 1.0f;

      QString units = TonometerMeasurement::unitsLUT.contains(key) ?
                  TonometerMeasurement::unitsLUT[key] : QString();
      setAttribute(key, Measurement::Value(value,units));

      qDebug() << QString("%1: %2").arg(key,getAttribute(key).toString());
    }
}

QString TonometerMeasurement::toString() const
{
    QString s;
    if(isValid())
    {
        //TODO: print implementation for all attributes
    }
    return s;
}

QDebug operator<<(QDebug dbg, const TonometerMeasurement &item)
{
    const QString s = item.toString();
    if (s.isEmpty())
        dbg.nospace() << "Tonometer Measurement()";
    else
        dbg.nospace() << "Tonometer Measurement(" << s << " ...)";
    return dbg.maybeSpace();
}
