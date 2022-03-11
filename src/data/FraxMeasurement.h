#ifndef FRAXMEASUREMENT_H
#define FRAXMEASUREMENT_H

#include "Measurement.h"

/*!
* \class FraxMeasurement
* \brief A Frax Measurement class
*
* Frax measurements are derived from output.txt file produced by Frax blackbox.exe
* This class facilitates converting the given inputs to output.
*
* \sa Measurement
*/

class FraxMeasurement : public Measurement
{
public:
    FraxMeasurement() = default;
    ~FraxMeasurement() = default;

    void fromString(const QString &);

    bool isValid() const override;

    QString toString() const override;
};

Q_DECLARE_METATYPE(FraxMeasurement);

QDebug operator<<(QDebug dbg, const FraxMeasurement&);

#endif // FRAXMEASUREMENT_H
