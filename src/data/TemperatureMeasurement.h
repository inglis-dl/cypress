#ifndef TEMPERATUREMEASUREMENT_H
#define TEMPERATUREMEASUREMENT_H

#include "MeasurementBase.h"

/*!
 * \class TemperatureMeasurement
 * \brief A TemperatureMeasurement class
 *
 * Measurements of temperature are derived from a
 * Masimo TIR-1 non-contact thermometer using bluetooth low energy
 * communication.  This class facilitates parsing
 * QByteArray input from the the BluetoothLEManager
 * class into measurement characteristics, such as value, units etc.
 *
 * \sa MeasurementBase
 */

class TemperatureMeasurement :  public MeasurementBase
{   
public:
    void fromArray(const QByteArray &);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    bool isValid() const override;

    static TemperatureMeasurement simulate();
};

Q_DECLARE_METATYPE(TemperatureMeasurement);

QDebug operator<<(QDebug dbg, const MeasurementBase &);

#endif // TEMPERATUREMEASUREMENT_H
