#ifndef TRUEFLOWSPIROMETERMEASUREMENT_H
#define TRUEFLOWSPIROMETERMEASUREMENT_H

#include "../../src/data/MeasurementBase.h"

/*!
* \class TrueFlowSpirometerMeasurement
* \brief A TrueFlowSpirometerMeasurement class
*
* \sa MeasurementBase
*/

class TrueFlowSpirometerMeasurement : public MeasurementBase
{
public:
    TrueFlowSpirometerMeasurement() = default;
    ~TrueFlowSpirometerMeasurement() = default;

    void fromString(const QString&);

    bool isValid() const override;

    QString toString() const override;

    static TrueFlowSpirometerMeasurement simulate();
};

Q_DECLARE_METATYPE(TrueFlowSpirometerMeasurement);

QDebug operator<<(QDebug dbg, const TrueFlowSpirometerMeasurement&);

#endif // TRUEFLOWSPIROMETERMEASUREMENT_H


