#ifndef TEMPERATURETEST_H
#define TEMPERATURETEST_H

#include "TestBase.h"
#include "TemperatureMeasurement.h"

/*!
 * \class TemperatureTest
 * \brief A Masimo TIR-1 bluetooth thermometer test class
 *
 * Concrete implementation of TestBase using TemperatureMeasurement
 * class specialization.
 *
 * \sa BluetoothLEManager, TemperatureMeasurement, Measurement
 *
 */

class TemperatureTest : public TestBase<TemperatureMeasurement>
{
public:
    TemperatureTest() = default;
    ~TemperatureTest() = default;

    void fromArray( const QByteArray &);

    QString toString() const override;

    bool isValid() const override;

    QJsonObject toJsonObject() const override;

};

Q_DECLARE_METATYPE(TemperatureTest);

#endif // TEMPERATURETEST_H
