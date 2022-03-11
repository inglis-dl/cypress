#ifndef BODYCOMPOSITIONMEASUREMENT_H
#define BODYCOMPOSITIONMEASUREMENT_H

#include "Measurement.h"

class BodyCompositionMeasurement : public Measurement
{
public:
    BodyCompositionMeasurement() = default;
    ~BodyCompositionMeasurement() = default;

    bool isValid() const override;

    static QStringList variableList;
    static QStringList initVariableList();
};

Q_DECLARE_METATYPE(BodyCompositionMeasurement);

QDebug operator<<(QDebug dbg, const BodyCompositionMeasurement &);

#endif // BODYCOMPOSITIONMEASUREMENT_H
