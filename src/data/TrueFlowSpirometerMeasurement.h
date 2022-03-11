#ifndef TRUEFLOWSPIROMETERMEASUREMENT_H
#define TRUEFLOWSPIROMETERMEASUREMENT_H

#include "../../src/data/Measurement.h"
#include "../prototype//TrueFlowSpirometer/Models/TrialDataModel.h"

/*!
* \class TrueFlowSpirometerMeasurement
* \brief A TrueFlowSpirometerMeasurement class
*
* \sa Measurement
*/

class TrueFlowSpirometerMeasurement : public Measurement
{
public:
    TrueFlowSpirometerMeasurement() = default;
    ~TrueFlowSpirometerMeasurement() = default;

    void fromTrialData(const TrialDataModel&);

    bool isValid() const override;

    QString toString() const override;

    static TrueFlowSpirometerMeasurement simulate();
};

Q_DECLARE_METATYPE(TrueFlowSpirometerMeasurement);

QDebug operator<<(QDebug dbg, const TrueFlowSpirometerMeasurement&);

#endif // TRUEFLOWSPIROMETERMEASUREMENT_H


