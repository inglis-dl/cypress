#ifndef CDTTMEASUREMENT_H
#define CDTTMEASUREMENT_H

#include "Measurement.h"

/*!
* \class CDTTTMeasurement
* \brief A CDTTTMeasurement class
*
* Measurements of CDTTT are derived from a headset and
* number keypad running the CDTTTstereo jar.
* This class facilitates parsing comma delimited data from
* the xlsx output of a test run.
*
* \sa Measurement
*/

class CDTTMeasurement : public Measurement
{
public:
    CDTTMeasurement() = default;
    ~CDTTMeasurement() = default;

    bool isValid() const override;

    void simulate(const quint16&);

    QString toString() const override;
};

Q_DECLARE_METATYPE(CDTTMeasurement);

QDebug operator<<(QDebug dbg, const CDTTMeasurement &);

#endif // CDTTMEASUREMENT_H
