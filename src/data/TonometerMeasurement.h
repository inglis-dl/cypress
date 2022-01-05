#ifndef TONOMETERMEASUREMENT_H
#define TONOMETERMEASUREMENT_H

#include "MeasurementBase.h"

/*!
* \class TonometerMeasurement
* \brief A Tonometer Measurement class
*
* Tonometer measurements are derived from output.txt file produced by Tonometer blackbox.exe
* This class facilitates converting the given inputs to output.
*
* \sa MeasurementBase
*/

class TonometerMeasurement : public MeasurementBase
{
public:
    TonometerMeasurement() = default;
    ~TonometerMeasurement() = default;

    void fromString(const QString &);

    bool isValid() const override;

    QString toString() const override;
};

Q_DECLARE_METATYPE(TonometerMeasurement);

QDebug operator<<(QDebug dbg, const TonometerMeasurement&);

#endif // TONOMETERMEASUREMENT_H
