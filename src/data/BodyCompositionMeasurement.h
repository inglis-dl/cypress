#ifndef BODYCOMPOSITIONMEASUREMENT_H
#define BODYCOMPOSITIONMEASUREMENT_H

#include "MeasurementBase.h"

class BodyCompositionMeasurement : public MeasurementBase
{
public:
    BodyCompositionMeasurement() = default;
    ~BodyCompositionMeasurement() = default;

    QString toString() const override;

    bool isValid() const override;
};

Q_DECLARE_METATYPE(BodyCompositionMeasurement);

QDebug operator<<(QDebug dbg, const BodyCompositionMeasurement &);

#endif // BODYCOMPOSITIONMEASUREMENT_H
