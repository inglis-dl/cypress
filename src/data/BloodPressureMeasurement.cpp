#include "BloodPressureMeasurement.h"

#include <QDebug>
#include <QDateTime>
#include <QRandomGenerator>

BloodPressureMeasurement::BloodPressureMeasurement(
        const int& readingNum,
        const int &sbp, const int& dbp, const int& pulse,
        const QDateTime &start, const QDateTime& end)
{
    setAttribute("reading_number",readingNum);
    setAttribute("systolic",sbp,"mmHg");
    setAttribute("diastolic",dbp,"mmHg");
    setAttribute("pulse",pulse,"bpm");
    setAttribute("start_time",start);
    setAttribute("end_time",end);
}

bool BloodPressureMeasurement::isValid() const
{
    bool hasAllRequiredAttributes =
        hasAttribute("systolic")
        && hasAttribute("diastolic")
        && hasAttribute("pulse")
        && hasAttribute("start_time")
        && hasAttribute("end_time")
        && hasAttribute("reading_number");
    if(!hasAllRequiredAttributes)
    {
      return false;
    }

    // TODO: There should be some low and high values which
    //       are not feasible to be blood pressures. These 
    //       values should be set as boundries for check
    //       normal systolic pressure: 90 - 120 mmHg
    //       normal diastolic pressure: 60 - 80 mmHg
    //       normal pulse rate: 60 - 100 bpm
    //
    bool sbpValid = 0 < getSbp();
    bool dbpValid = 0 < getDbp();
    bool pulseValid = 0 < getPulse();
    return (sbpValid && dbpValid && pulseValid);
}

QString BloodPressureMeasurement::toString() const
{
    return QString("%1. SBP: %2 DBP: %3 Pulse: %4 (%5 -> %6)").arg(
        getAttributeValue("reading_number").toString(),
        getAttributeValue("systolic").toString(),
        getAttributeValue("diastolic").toString(),
        getAttributeValue("pulse").toString(),
        getAttributeValue("start_time").toString(),
        getAttributeValue("end_time").toString());
}

BloodPressureMeasurement BloodPressureMeasurement::simulate(const int& reading)
{
    static QDateTime previous = QDateTime::currentDateTime();

    int systolic = QRandomGenerator::global()->bounded(90,120);
    int diastolic = QRandomGenerator::global()->bounded(60,80);
    int pulse = QRandomGenerator::global()->bounded(60,100);
    int timeoffset = QRandomGenerator::global()->bounded(26000,99000);
    QDateTime start = QDateTime::currentDateTime();
    if(start<previous) start = previous;
    QDateTime end = start.addMSecs(timeoffset);
    previous = end;
    BloodPressureMeasurement measure(reading, systolic, diastolic, pulse, start, end);
    qDebug() << measure.toString();
    return measure;
}

QDebug operator<<(QDebug dbg, const BloodPressureMeasurement& item)
{
    const QString measurementStr = item.toString();
    if (measurementStr.isEmpty())
        dbg.nospace() << "Blood Pressure Measurement()";
    else
        dbg.nospace() << "Blood Pressure Measurement(" << measurementStr << " ...)";
    return dbg.maybeSpace();
}
