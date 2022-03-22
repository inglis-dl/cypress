#ifndef ECGMEASUREMENT_H
#define ECGMEASUREMENT_H

#include "Measurement.h"

/*!
* \class ECGMeasurement
* \brief A ECG Measurement class
*
* ECG measurements are derived from output.txt file produced by ECG blackbox.exe
* This class facilitates converting the given inputs to output.
*
* \sa Measurement
*/

class ECGMeasurement : public Measurement
{
public:
    ECGMeasurement() = default;
    ~ECGMeasurement() = default;

    void fromString(const QString &);

    bool isValid() const override;

    QString toString() const override;
};

Q_DECLARE_METATYPE(ECGMeasurement);

QDebug operator<<(QDebug dbg, const ECGMeasurement&);

#endif // ECGMEASUREMENT_H
