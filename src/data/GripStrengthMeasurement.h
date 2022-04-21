#ifndef GRIPSTRENGTHMEASUREMENT_H
#define GRIPSTRENGTHMEASUREMENT_H

#include "../../src/data/Measurement.h"

/*!
* \class GripStrengthMeasurement
* \brief A GripStrengthMeasurement class
*
* \sa Measurement
*/

class GripStrengthMeasurement : public Measurement
{
public:
    GripStrengthMeasurement() = default;
    ~GripStrengthMeasurement() = default;

    void fromString(const QString& string) { };

    bool isValid() const override;

    QString toString() const override;

    static GripStrengthMeasurement simulate();
};

Q_DECLARE_METATYPE(GripStrengthMeasurement);

QDebug operator<<(QDebug dbg, const GripStrengthMeasurement&);

#endif // GRIPSTRENGTHMEASUREMENT_H


