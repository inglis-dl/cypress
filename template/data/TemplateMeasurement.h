#ifndef TEMPLATEMEASUREMENT_H
#define TEMPLATEMEASUREMENT_H

#include "../../src/data/Measurement.h"

/*!
* \class TemplateMeasurement
* \brief A TemplateMeasurement class
*
* \sa Measurement
*/

class TemplateMeasurement : public Measurement
{
public:
    TemplateMeasurement() = default;
    ~TemplateMeasurement() = default;

    void fromString(const QString& string) { };

    bool isValid() const override;

    QString toString() const override;

    static TemplateMeasurement simulate();
};

Q_DECLARE_METATYPE(TemplateMeasurement);

QDebug operator<<(QDebug dbg, const TemplateMeasurement&);

#endif // TEMPLATEMEASUREMENT_H


