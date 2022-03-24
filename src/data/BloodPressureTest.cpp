#include "BloodPressureTest.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

// the minimum output data keys required from a successful a test
//
BloodPressureTest::BloodPressureTest()
{
    m_outputKeyList << "cuff_size";
    m_outputKeyList << "side";

    m_outputKeyList << "first_systolic";
    m_outputKeyList << "first_diastolic";
    m_outputKeyList << "first_pulse";
    m_outputKeyList << "first_start_time";
    m_outputKeyList << "first_end_time";

    m_outputKeyList << "avg_systolic";
    m_outputKeyList << "avg_diastolic";
    m_outputKeyList << "avg_pulse";
    m_outputKeyList << "avg_count";

    m_outputKeyList << "total_avg_systolic";
    m_outputKeyList << "total_avg_diastolic";
    m_outputKeyList << "total_avg_pulse";
    m_outputKeyList << "total_avg_count";
}

// String representation for debug and GUI display purposes
//
QString BloodPressureTest::toString() const
{
    QString str;
    if(isValid())
    {
        QStringList list;
        foreach(const auto m, m_measurementList)
        {
          list << m.toString();
        }
        str = list.join("\n");
    }
    return str;
}

void BloodPressureTest::simulate()
{
    int count = getMaximumNumberOfMeasurements();
    int systolicAvg = 0;
    int diastolicAvg = 0;
    int pulseAvg = 0;
    for(int i = 0; i < count; i++)
    {
      BloodPressureMeasurement m = BloodPressureMeasurement::simulate(i+1);
      addMeasurement(m);
      if(0 == i) continue;
      systolicAvg += m.getSbp();
      diastolicAvg += m.getDbp();
      pulseAvg += m.getPulse();
    }
    systolicAvg = qRound(systolicAvg * 1.0f/(count-1));
    diastolicAvg = qRound(diastolicAvg * 1.0f/(count-1));
    pulseAvg = qRound(pulseAvg * 1.0f/(count-1));

    addDeviceAverage(systolicAvg,diastolicAvg,pulseAvg);
}

bool BloodPressureTest::isValid() const
{
    bool okMeta = true;
    foreach(const auto key, m_outputKeyList)
    {
      if(!hasMetaData(key))
      {
         okMeta = false;
         break;
       }
    }
    bool okTest = 1 <= getNumberOfMeasurements();
    if(okTest)
    {
      foreach(const auto m, m_measurementList)
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

// String keys are converted to snake_case
//
QJsonObject BloodPressureTest::toJsonObject() const
{
    QJsonArray jsonArr;
    foreach(const auto m, m_measurementList)
    {
      jsonArr.append(m.toJsonObject());
    }
    QJsonObject json;
    if(hasMetaData())
      json.insert("test_meta_data",m_metaData.toJsonObject());
    json.insert("test_results",jsonArr);
    return json;
}

bool BloodPressureTest::verifyDeviceAverage(const int& sbp, const int& dbp, const int& pulse) const
{
    bool dataMatches = false;
    if(hasAverage())
    {
        int avgSystolic = getMetaData("avg_systolic").toInt();
        int avgDiastolic = getMetaData("avg_diastolic").toInt();
        int avgPulse = getMetaData("avg_pulse").toInt();
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

// add the average of the measurements provided by the device and compare
// to computed averages from the preceding stored individual measurements
//
void BloodPressureTest::addDeviceAverage(const int& sbpAvg, const int& dbpAvg, const int& pulseAvg)
{
    int sbpTotal = 0;
    int dbpTotal = 0;
    int pulseTotal = 0;
    if(m_measurementList.isEmpty())
    {
      qDebug() << "No measurements to average";
      return;
    }
    // add the first measurement as separate test meta data
    BloodPressureMeasurement first = m_measurementList.first();
    addMetaData("first_systolic",first.getAttribute("systolic"));
    addMetaData("first_diastolic",first.getAttribute("diastolic"));
    addMetaData("first_pulse",first.getAttribute("pulse"));
    addMetaData("first_start_time",first.getAttribute("start_time"));
    addMetaData("first_end_time",first.getAttribute("end_time"));

    // skip the first measurement
    int count = 0;
    for(int i = 1; i < m_measurementList.count(); i++)
    {
      BloodPressureMeasurement measurement = m_measurementList[i];
      if(measurement.isValid())
      {
        count++;
        sbpTotal += measurement.getSbp();
        dbpTotal += measurement.getDbp();
        pulseTotal += measurement.getPulse();
        qDebug() << QString("sbpTotal = %1 dbpTotal = %2 pulseTotal = %3").arg(sbpTotal).arg(dbpTotal).arg(pulseTotal);
      }
    }
    if(1 > count)
    {
        qDebug() << "ERROR: not enough measurements to validated device contributed averages";
        return;
    }
    double avgSbpCalc = sbpTotal * 1.0f / count;
    double avgDbpCalc = dbpTotal * 1.0f / count;
    double avgPulseCalc = pulseTotal * 1.0f / count;

    addMetaData("avg_count", QVariant(count));

    qDebug() << QString("Averages: sbp(%1:%2) dbp(%3:%4) pulse(%5:%6)").arg(sbpAvg).arg(avgSbpCalc).arg(dbpAvg).arg(avgDbpCalc).arg(pulseAvg).arg(avgPulseCalc);

    bool ok = true;
    if(qRound(avgSbpCalc) == sbpAvg)
    {
      addMetaData("avg_systolic",sbpAvg,"mmHg");
    }
    else
    {
      qDebug() << QString("WARNING: SBP average (%1) does not align with calculated average (%2)").arg(sbpAvg).arg(avgSbpCalc);
      ok = false;
    }
    if(qRound(avgDbpCalc) == dbpAvg)
    {
      addMetaData("avg_diastolic",dbpAvg,"mmHg");
    }
    else
    {
      qDebug() << QString("WARNING: DBP average (%1) does not align with calculated average (%2)").arg(dbpAvg).arg(avgDbpCalc);
      ok = false;
    }
    if(qRound(avgPulseCalc) == pulseAvg)
    {
      addMetaData("avg_pulse",pulseAvg,"bpm");
    }
    else
    {
      qDebug() << QString("WARNING: Pulse average (%1) does not align with calculated average (%2)").arg(pulseAvg).arg(avgPulseCalc);
      ok = false;
    }

    if(ok)
      computeTotalAverage(sbpTotal, dbpTotal, pulseTotal);
}

void BloodPressureTest::computeTotalAverage(int sbpTotal, int dbpTotal, int pulseTotal)
{
    if(hasFirstMeasurement())
    {
      int count = m_measurementList.count() + 1;
      sbpTotal += getMetaData("first_systolic").toInt();
      dbpTotal += getMetaData("first_diastolic").toInt();
      pulseTotal += getMetaData("first_pulse").toInt();
      addMetaData("total_avg_systolic",qRound(sbpTotal * 1.0f / count),"mmHg");
      addMetaData("total_avg_diastolic",qRound(dbpTotal * 1.0f / count),"mmHg");
      addMetaData("total_avg_pulse",qRound(pulseTotal * 1.0f / count),"bpm");
      addMetaData("total_avg_count",count);
    }
    else
    {
      qDebug() << "WARNING: No data found for first measurement when trying to calculate all average data";
    }
}

bool BloodPressureTest::hasFirstMeasurement() const
{
  return hasMetaData("first_start_time") &&
         hasMetaData("first_end_time") &&
         hasMetaData("first_systolic") &&
         hasMetaData("first_diastolic") &&
         hasMetaData("first_pulse");
}

bool BloodPressureTest::hasAverage() const
{
  return hasMetaData("avg_systolic") &&
         hasMetaData("avg_diastolic") &&
         hasMetaData("avg_pulse") &&
         hasMetaData("avg_count");
}

bool BloodPressureTest::hasTotalAverage() const
{
  return hasMetaData("total_avg_systolic") &&
         hasMetaData("total_avg_diastolic") &&
         hasMetaData("total_avg_pulse")&&
         hasMetaData("total_avg_count");
}

void BloodPressureTest::setCuffSize(const QString &size)
{
    addMetaData("cuff_size", QVariant(size));
}

void BloodPressureTest::setSide(const QString &side)
{
    addMetaData("side", QVariant(side));
}

bool BloodPressureTest::armInformationSet() const
{
    return hasMetaData("cuff_size")
        && hasMetaData("side");
}

void BloodPressureTest::reset()
{
    QList<QString> keys = m_outputKeyList;
    keys.removeAll("cuff_size");
    keys.removeAll("side");
    m_metaData.remove(keys);
    m_measurementList.clear();
}
