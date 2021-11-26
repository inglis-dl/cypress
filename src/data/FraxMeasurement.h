#ifndef FRAXMEASUREMENT_H
#define FRAXMEASUREMENT_H

#include "MeasurementBase.h"

/*!
* \class FraxMeasurement
* \brief A FraxMeasurement class
*
* Measurements of Frax are derived from inputs
* passed in from juniper.
* This class facilitates converting the given inputs to output.
*
* \sa MeasurementBase
*/

class FraxMeasurement : public MeasurementBase
{
public:
    FraxMeasurement() = default;
    ~FraxMeasurement() = default;

    void fromString(const QString&);

    bool isValid() const override;

    QString toString() const override;
};

Q_DECLARE_METATYPE(FraxMeasurement);

QDebug operator<<(QDebug dbg, const FraxMeasurement&);

#endif // FRAXMEASUREMENT_H