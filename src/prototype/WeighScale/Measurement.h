#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include "MeasurementBase.h"

/*!
 * \class Measurement
 * \brief A Measurement class
 *
 * Measurements of weight are derived from a
 * Rice Lake digital weigh scale over RS232 serial
 * communication.  This class facilitates parsing
 * QByteArray input from the QSerialPort of the WeighScaleManager
 * class into measurement characteristics, such as value, units etc.
 *
 * \sa MeasurementBase
 */

class Measurement :  public MeasurementBase
{   
public:
    void fromArray(const QByteArray &);

    QString toString() const;

    bool isValid() const;

    bool isZero() const;

protected:
};

Q_DECLARE_METATYPE(Measurement);

#endif // MEASUREMENT_H
