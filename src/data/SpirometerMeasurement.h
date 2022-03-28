#ifndef SPIROMETERMEASUREMENT_H
#define SPIROMETERMEASUREMENT_H

#include "Measurement.h"

#include "../managers/EMRPluginHelper.h"
/*!
* \class SpirometerMeasurement
* \brief A SpirometerMeasurement class
*
* \sa Measurement
*/

class SpirometerMeasurement : public Measurement
{
public:
    SpirometerMeasurement() = default;
    ~SpirometerMeasurement() = default;

    typedef typename EMRPluginHelper::TrialDataModel trialData;
    typedef typename EMRPluginHelper::ResultParametersModel parameterData;

    void fromTrialData(const trialData&);

    bool isValid() const override;

    QString toString() const override;

    QStringList getHeaderValues() const;

    static SpirometerMeasurement simulate();

    parameterData getMetaResults() { return metaResults; }

private:
    parameterData metaResults;
    void appendMeasurementAttribute(QString* measurementStr, const QString& key) const;
};

Q_DECLARE_METATYPE(SpirometerMeasurement);

QDebug operator<<(QDebug dbg, const SpirometerMeasurement&);

#endif // SPIROMETERMEASUREMENT_H


