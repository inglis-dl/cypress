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

QT_FORWARD_DECLARE_CLASS(QDomNode)

class ECGMeasurement : public Measurement
{
public:
    ECGMeasurement() = default;
    ~ECGMeasurement() = default;

    void fromDomNode(const QDomNode&);

    bool isValid() const override;

    QString toString() const override;

    void simulate();

private:

    void readInterpretation(const QDomNode&);
    void readRestingECGMeasurements(const QDomNode&);
};

Q_DECLARE_METATYPE(ECGMeasurement);

QDebug operator<<(QDebug dbg, const ECGMeasurement&);

#endif // ECGMEASUREMENT_H
