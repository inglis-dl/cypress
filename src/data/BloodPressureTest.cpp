#include "BloodPressureTest.h"

#include <QDateTime>
#include <QJsonArray>
#include <QFileInfo>
#include <QDebug>

// the minimum output data keys required from a successful a test
//
BloodPressureTest::BloodPressureTest()
{
    m_outputKeyList << "user id";
}

void BloodPressureTest::addMeasurement(const int& sbp, const int& dbp, const int& pulse, const const QDateTime& start, const QDateTime& end, const int& readingNum)
{
    if (readingNum == 1) {
        addMetaDataCharacteristic("first start time", start);
        addMetaDataCharacteristic("first end time", end);
        addMetaDataCharacteristic("first systolic", sbp);
        addMetaDataCharacteristic("first diastolic", dbp);
        addMetaDataCharacteristic("first pulse", pulse);
        return;
    }

    BloodPressureMeasurement measurement;
    measurement.storeData(sbp, dbp, pulse, start, end, readingNum);
    m_measurementList.append(measurement);
}

// String representation for debug and GUI display purposes
//
QString BloodPressureTest::toString() const
{
    QString outputStr;
    if (isValid())
    {
        QStringList tempList;
        tempList << firstMeasurementToString();
        
        for (auto&& measurement : m_measurementList)
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
    // Check meta data is correct
    bool okMeta = hasFirstMeasurementData()
        && hasAvgMeasurementData()
        && hasAllAvgMeasurementData()
        && armInformationSet();

    if (false == okMeta) {
        return false;
    }

    // Check number of measurements is correct
    bool okTest = 5 == getNumberOfMeasurements();
    if (false == okTest) {
        return false;
    }

    for (auto&& measurement : m_measurementList)
    {
        if (false == measurement.isValid()) {
            return false;
        }
    }
    return true;
}

// String keys are converted to snake_case
//
QJsonObject BloodPressureTest::toJsonObject() const
{
    QJsonArray jsonArr;
    for (auto&& measurement : m_measurementList)
    {
        jsonArr.append(measurement.toJsonObject());
    }
    QJsonObject json;
    json.insert("test_meta_data", m_metaData.toJsonObject());
    json.insert("test_results", jsonArr);
    return json;
}

bool BloodPressureTest::verifyReviewData(const int sbp, const int dbp, const int pulse)
{
    bool dataMatches = false;
    if (hasAvgMeasurementData()) {
        int avgSystolic = getMetaDataCharacteristic("avg systolic").toInt();
        int avgDiastolic = getMetaDataCharacteristic("avg diastolic").toInt();
        int avgPulse = getMetaDataCharacteristic("avg pulse").toInt();
        if (avgSystolic == sbp && avgDiastolic == dbp && avgPulse == pulse) {
            dataMatches = true;
        }
        else {
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
    if (0 >= numMeasurements) {
        qDebug() << "No measurements to average";
        return;
    }
    for (int i = 0; i < numMeasurements; i++) {
        BloodPressureMeasurement measurement = m_measurementList[i];
        if (measurement.isValid()) {
            sbpTotal += measurement.getSbp();
            dbpTotal += measurement.getDbp();
            pulseTotal += measurement.getPulse();
            qDebug() << QString("sbpTotal = %1 dbpTotal = %2 pulseTotal = %3").arg(sbpTotal).arg(dbpTotal).arg(pulseTotal);
        }
    }
    double avgSbpCalc = sbpTotal * 1.0 / numMeasurements;
    double avgDbpCalc = dbpTotal * 1.0 / numMeasurements;
    double avgPulseCalc = pulseTotal * 1.0 / numMeasurements;

    addMetaDataCharacteristic("avg count", numMeasurements);

    qDebug() << QString("Averages: sbp(%1:%2) dbp(%3:%4) pulse(%5:%6)").arg(sbpAvg).arg(avgSbpCalc).arg(dbpAvg).arg(avgDbpCalc).arg(pulseAvg).arg(avgPulseCalc);
    if (round(avgSbpCalc) == sbpAvg) {
        addMetaDataCharacteristic("avg systolic", sbpAvg);
    }else {
        qDebug() << QString("WARNING: SBP average (%1) does not align with calculated average (%2)").arg(sbpAvg).arg(avgSbpCalc);
    }

    if (round(avgDbpCalc) == dbpAvg) {
        addMetaDataCharacteristic("avg diastolic", dbpAvg);
    }else{
        qDebug() << QString("WARNING: DBP average (%1) does not align with calculated average (%2)").arg(dbpAvg).arg(avgDbpCalc);
    }

    if (round(avgPulseCalc) == pulseAvg) {
        addMetaDataCharacteristic("avg pulse", pulseAvg);
    }else {
        qDebug() << QString("WARNING: Pulse average (%1) does not align with calculated average (%2)").arg(pulseAvg).arg(avgPulseCalc);
    }

    storeAllAverageMetaData(sbpTotal, dbpTotal, pulseTotal);
}

void BloodPressureTest::storeAllAverageMetaData(int sbpTotal, int dbpTotal, int pulseTotal)
{
    if (hasFirstMeasurementData()) {
        int numMeasurments = m_measurementList.count() + 1;
        sbpTotal += getMetaDataCharacteristic("first systolic").toInt();
        dbpTotal += getMetaDataCharacteristic("first diastolic").toInt();
        pulseTotal += getMetaDataCharacteristic("first pulse").toInt();
        addMetaDataCharacteristic("all avg systolic", round(sbpTotal * 1.0 / numMeasurments));
        addMetaDataCharacteristic("all avg diastolic", round(dbpTotal * 1.0 / numMeasurments));
        addMetaDataCharacteristic("all avg pulse", round(pulseTotal * 1.0 / numMeasurments));
    }
    else {
        qDebug() << "WARNING: No data found for first measurement when trying to calculate all average data";
    }
}

bool BloodPressureTest::hasFirstMeasurementData() const
{
    return hasMetaDataCharacteristic("first start time") &&
        hasMetaDataCharacteristic("first end time") &&
        hasMetaDataCharacteristic("first systolic") &&
        hasMetaDataCharacteristic("first diastolic") &&
        hasMetaDataCharacteristic("first pulse");
}

bool BloodPressureTest::hasAvgMeasurementData() const
{
    return hasMetaDataCharacteristic("avg systolic") &&
        hasMetaDataCharacteristic("avg diastolic") &&
        hasMetaDataCharacteristic("avg pulse") &&
        hasMetaDataCharacteristic("avg count");
}

bool BloodPressureTest::hasAllAvgMeasurementData() const
{
    return hasMetaDataCharacteristic("all avg systolic") &&
        hasMetaDataCharacteristic("all avg diastolic") &&
        hasMetaDataCharacteristic("all avg pulse");
}

QString BloodPressureTest::firstMeasurementToString() const
{
    if (false == hasFirstMeasurementData()) {
        return "";
    }

    return QString("1. SBP: %1 DBP: %2 Pulse: %3 (%4 -> %5)")
        .arg(getMetaDataCharacteristic("first systolic").toInt())
        .arg(getMetaDataCharacteristic("first diastolic").toInt())
        .arg(getMetaDataCharacteristic("first pulse").toInt())
        .arg(getMetaDataCharacteristic("first start time").toDateTime().toString("yyyy-MM-dd  HH:mm:ss"))
        .arg(getMetaDataCharacteristic("first end time").toDateTime().toString("HH:mm:ss"));
}

QString BloodPressureTest::avgMeasurementToString() const
{
    if (false == hasAvgMeasurementData()) {
        return "";
    }

    return QString("AVG. SBP: %1 DBP: %2 Pulse: %3")
        .arg(getMetaDataCharacteristic("avg systolic").toInt())
        .arg(getMetaDataCharacteristic("avg diastolic").toInt())
        .arg(getMetaDataCharacteristic("avg pulse").toInt());
}

QString BloodPressureTest::allAvgMeasurementToString() const
{
    if (false == hasAllAvgMeasurementData()) {
        return "";
    }

    return QString("All AVG. SBP: %1 DBP: %2 Pulse: %3")
        .arg(getMetaDataCharacteristic("all avg systolic").toInt())
        .arg(getMetaDataCharacteristic("all avg diastolic").toInt())
        .arg(getMetaDataCharacteristic("all avg pulse").toInt());
}

bool BloodPressureTest::armInformationSet() const
{
    return hasMetaDataCharacteristic("arm band size")
        && hasMetaDataCharacteristic("arm used")
        && getMetaDataCharacteristic("arm band size") != ""
        && getMetaDataCharacteristic("arm used") != "";
}
