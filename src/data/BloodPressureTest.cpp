#include "BloodPressureTest.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
//#include <QFileInfo>
#include <QDebug>

// the minimum output data keys required from a successful a test
//
BloodPressureTest::BloodPressureTest()
{
    m_outputKeyList << "cuff_size";
    m_outputKeyList << "side";
    m_outputKeyList << "first_start_time";
    m_outputKeyList << "first_end_time";
    m_outputKeyList << "first_systolic";
    m_outputKeyList << "first_diastolic";
    m_outputKeyList << "first_pulse";
    m_outputKeyList << "avg_count";
    m_outputKeyList << "avg_systolic";
    m_outputKeyList << "avg_diastolic";
    m_outputKeyList << "avg_pulse";
    m_outputKeyList << "all_avg_systolic";
    m_outputKeyList << "all_avg_diastolic";
    m_outputKeyList << "all_avg_pulse";
}

void BloodPressureTest::addMeasurement(const int& sbp, const int& dbp,
                                       const int& pulse,
                                       const QDateTime& start,
                                       const QDateTime& end,
                                       const int& readingNum)
{
    if(1 == readingNum)
    {
        addMetaDataCharacteristic("first_start_time", start);
        addMetaDataCharacteristic("first_end_time", end);
        addMetaDataCharacteristic("first_systolic", sbp);
        addMetaDataCharacteristic("first_diastolic", dbp);
        addMetaDataCharacteristic("first_pulse", pulse);
        return;
    }

    BloodPressureMeasurement measurement(sbp, dbp, pulse, start, end, readingNum);
    m_measurementList.append(measurement);
}

// String representation for debug and GUI display purposes
//
QString BloodPressureTest::toString() const
{
    QString outputStr;
    if(isValid())
    {
        QStringList tempList;
        tempList << firstMeasurementToString();
        
        for(auto&& measurement : m_measurementList)
        {
          tempList << measurement.toString();
        }

        tempList << avgMeasurementToString();
        tempList << allAvgMeasurementToString();
        outputStr = tempList.join("\n");
    }
    return outputStr;
}

bool BloodPressureTest::isValid() const
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

    bool okTest = 5 == getNumberOfMeasurements();
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
QJsonObject BloodPressureTest::toJsonObject() const
{
    QJsonArray jsonArr;
    for(auto&& measurement : m_measurementList)
    {
        jsonArr.append(measurement.toJsonObject());
    }
    QJsonObject json;
    json.insert("test_meta_data", m_metaData.toJsonObject());
    json.insert("test_results", jsonArr);
    return json;
}

bool BloodPressureTest::verifyReviewData(const int& sbp, const int& dbp, const int& pulse) const
{
    bool dataMatches = false;
    if(hasAvgMeasurementData())
    {
        int avgSystolic = getMetaDataCharacteristic("avg_systolic").toInt();
        int avgDiastolic = getMetaDataCharacteristic("avg_diastolic").toInt();
        int avgPulse = getMetaDataCharacteristic("avg_pulse").toInt();
        if(avgSystolic == sbp && avgDiastolic == dbp && avgPulse == pulse)
        {
          dataMatches = true;
        }
        else
        {
          qDebug() << QString("WARNING: Review data (sbp: %1, dbp: %2, pulse: %3) does not match average data (sbp: %4, dbp: %5, pulse: %6).")
                .arg(sbp).arg(dbp).arg(pulse).arg(avgSystolic).arg(avgDiastolic).arg(avgPulse);
        }
    }
    return dataMatches;
}

void BloodPressureTest::addAverageMeasurement(const int& sbpAvg, const int& dbpAvg, const int& pulseAvg)
{
    int sbpTotal = 0;
    int dbpTotal = 0;
    int pulseTotal = 0;
    int numMeasurements = m_measurementList.count();
    if(0 >= numMeasurements)
    {
      qDebug() << "No measurements to average";
      return;
    }
    for(int i = 0; i < numMeasurements; i++)
    {
      BloodPressureMeasurement measurement = m_measurementList[i];
      if(measurement.isValid())
      {
        sbpTotal += measurement.getSbp();
        dbpTotal += measurement.getDbp();
        pulseTotal += measurement.getPulse();
        qDebug() << QString("sbpTotal = %1 dbpTotal = %2 pulseTotal = %3").arg(sbpTotal).arg(dbpTotal).arg(pulseTotal);
      }
    }
    double avgSbpCalc = sbpTotal * 1.0f / numMeasurements;
    double avgDbpCalc = dbpTotal * 1.0f / numMeasurements;
    double avgPulseCalc = pulseTotal * 1.0f / numMeasurements;

    addMetaDataCharacteristic("avg_count", numMeasurements);

    qDebug() << QString("Averages: sbp(%1:%2) dbp(%3:%4) pulse(%5:%6)").arg(sbpAvg).arg(avgSbpCalc).arg(dbpAvg).arg(avgDbpCalc).arg(pulseAvg).arg(avgPulseCalc);
    if(qRound(avgSbpCalc) == sbpAvg)
    {
      addMetaDataCharacteristic("avg_systolic", sbpAvg);
    }
    else
    {
      qDebug() << QString("WARNING: SBP average (%1) does not align with calculated average (%2)").arg(sbpAvg).arg(avgSbpCalc);
    }

    if(qRound(avgDbpCalc) == dbpAvg)
    {
      addMetaDataCharacteristic("avg_diastolic", dbpAvg);
    }
    else
    {
      qDebug() << QString("WARNING: DBP average (%1) does not align with calculated average (%2)").arg(dbpAvg).arg(avgDbpCalc);
    }

    if(qRound(avgPulseCalc) == pulseAvg)
    {
      addMetaDataCharacteristic("avg_pulse", pulseAvg);
    }
    else
    {
      qDebug() << QString("WARNING: Pulse average (%1) does not align with calculated average (%2)").arg(pulseAvg).arg(avgPulseCalc);
    }

    storeAllAverageMetaData(sbpTotal, dbpTotal, pulseTotal);
}

void BloodPressureTest::storeAllAverageMetaData(int sbpTotal, int dbpTotal, int pulseTotal)
{
    if(hasFirstMeasurementData())
    {
      int numMeasurments = m_measurementList.count() + 1;
      sbpTotal += getMetaDataCharacteristic("first_systolic").toInt();
      dbpTotal += getMetaDataCharacteristic("first_diastolic").toInt();
      pulseTotal += getMetaDataCharacteristic("first_pulse").toInt();
      addMetaDataCharacteristic("all_avg_systolic", qRound(sbpTotal * 1.0f / numMeasurments));
      addMetaDataCharacteristic("all_avg_diastolic", qRound(dbpTotal * 1.0f / numMeasurments));
      addMetaDataCharacteristic("all_avg_pulse", qRound(pulseTotal * 1.0f / numMeasurments));
    }
    else
    {
      qDebug() << "WARNING: No data found for first measurement when trying to calculate all average data";
    }
}

bool BloodPressureTest::hasFirstMeasurementData() const
{
  return hasMetaDataCharacteristic("first_start_time") &&
         hasMetaDataCharacteristic("first_end_time") &&
         hasMetaDataCharacteristic("first_systolic") &&
         hasMetaDataCharacteristic("first_diastolic") &&
         hasMetaDataCharacteristic("first_pulse");
}

bool BloodPressureTest::hasAvgMeasurementData() const
{
  return hasMetaDataCharacteristic("avg_systolic") &&
         hasMetaDataCharacteristic("avg_diastolic") &&
         hasMetaDataCharacteristic("avg_pulse") &&
         hasMetaDataCharacteristic("avg_count");
}

bool BloodPressureTest::hasAllAvgMeasurementData() const
{
  return hasMetaDataCharacteristic("all_avg_systolic") &&
         hasMetaDataCharacteristic("all_avg_diastolic") &&
         hasMetaDataCharacteristic("all_avg_pulse");
}

QString BloodPressureTest::firstMeasurementToString() const
{
  if(!hasFirstMeasurementData())
  {
    return QString("");
  }

  return QString("1. SBP: %1 DBP: %2 Pulse: %3 (%4 -> %5)")
        .arg(getMetaDataCharacteristic("first_systolic").toInt())
        .arg(getMetaDataCharacteristic("first_diastolic").toInt())
        .arg(getMetaDataCharacteristic("first_pulse").toInt())
        .arg(getMetaDataCharacteristic("first_start_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss"))
        .arg(getMetaDataCharacteristic("first_end_time").toDateTime().toString("HH:mm:ss"));
}

QString BloodPressureTest::avgMeasurementToString() const
{
  if(!hasAvgMeasurementData())
  {
    return QString("");
  }

  return QString("AVG. SBP: %1 DBP: %2 Pulse: %3")
        .arg(getMetaDataCharacteristic("avg_systolic").toInt())
        .arg(getMetaDataCharacteristic("avg_diastolic").toInt())
        .arg(getMetaDataCharacteristic("avg_pulse").toInt());
}

QString BloodPressureTest::allAvgMeasurementToString() const
{
  if(!hasAllAvgMeasurementData())
  {
    return QString("");
  }

  return QString("All AVG. SBP: %1 DBP: %2 Pulse: %3")
        .arg(getMetaDataCharacteristic("all_avg_systolic").toInt())
        .arg(getMetaDataCharacteristic("all_avg_diastolic").toInt())
        .arg(getMetaDataCharacteristic("all_avg_pulse").toInt());
}

void BloodPressureTest::setCuffSize(const QString &size)
{
  if(!size.isEmpty())
    addMetaDataCharacteristic("cuff_size", size.toLower());
}

void BloodPressureTest::setSide(const QString &side)
{
  if(!side.isEmpty())
    addMetaDataCharacteristic("side", side.toLower());
}

bool BloodPressureTest::armInformationSet() const
{
    return hasMetaDataCharacteristic("cuff_size")
        && hasMetaDataCharacteristic("side");
}
