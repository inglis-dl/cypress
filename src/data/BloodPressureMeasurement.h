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

    void storeData(const int sbp, const int dbp, const int pulse, const bool isAverage);

    bool isValid() const override;

    QString toString() const override;

    static BloodPressureMeasurement simulate();
private:
    int m_sbp = -1;
    int m_dbp = -1;
    int m_pulse = -1;
    bool m_isAverage = false;
};

Q_DECLARE_METATYPE(BloodPressureMeasurement);

QDebug operator<<(QDebug dbg, const BloodPressureMeasurement&);

#endif // BLOODPRESSUREMEASUREMENT_H


