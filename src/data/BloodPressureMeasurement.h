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

    void fromString(const QString&);

    bool isValid() const override;

    QString toString() const override;

    static BloodPressureMeasurement simulate();
};

Q_DECLARE_METATYPE(BloodPressureMeasurement);

QDebug operator<<(QDebug dbg, const BloodPressureMeasurement&);

#endif // BLOODPRESSUREMEASUREMENT_H


