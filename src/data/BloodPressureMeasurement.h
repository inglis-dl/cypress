#ifndef BLOODPRESSUREMEASUREMENT_H
#define BLOODPRESSUREMEASUREMENT_H

#include "MeasurementBase.h"

/*!
* \class BloodPressureMeasurement
* \brief A BloodPressureMeasurement class
*
* Measurements of Blood Pressure are derived from a blood pressure monitor
* This class facilitates reading byte data
*
* \sa MeasurementBase
*/

class BloodPressureMeasurement : public MeasurementBase
{
public:
    BloodPressureMeasurement() = default;
    ~BloodPressureMeasurement() = default;

    BloodPressureMeasurement(const int& readingNum,
            const int& sbp, const int& dbp,
            const int& pulse, const QDateTime& start,
            const QDateTime& end);

    bool isValid() const override;

    QString toString() const override;

    static BloodPressureMeasurement simulate(const int&);

    int getSbp() const { return getCharacteristic("systolic").toInt(); }
    int getDbp() const { return getCharacteristic("diastolic").toInt(); }
    int getPulse() const { return getCharacteristic("pulse").toInt(); }
};

Q_DECLARE_METATYPE(BloodPressureMeasurement);

QDebug operator<<(QDebug dbg, const BloodPressureMeasurement&);

#endif // BLOODPRESSUREMEASUREMENT_H


