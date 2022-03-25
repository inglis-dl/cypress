#ifndef SPIROMETERMEASUREMENT_H
#define SPIROMETERMEASUREMENT_H

#include "Measurement.h"
#include "TrialDataModel.h"

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

    void fromTrialData(const TrialDataModel&);

    bool isValid() const override;

    QString toString() const override;

    QStringList getHeaderValues() const;

    static SpirometerMeasurement simulate();

    ResultParametersModel getMetaResults() { return metaResults; }
private:
    ResultParametersModel metaResults;
    void appendMeasurementAttribute(QString* measurementStr, const QString& key) const;
};

Q_DECLARE_METATYPE(SpirometerMeasurement);

QDebug operator<<(QDebug dbg, const SpirometerMeasurement&);

#endif // SPIROMETERMEASUREMENT_H


