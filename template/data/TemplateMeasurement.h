#ifndef TEMPLATEMEASUREMENT_H
#define TEMPLATEMEASUREMENT_H

#include "../../src/data/MeasurementBase.h"

/*!
* \class TemplateMeasurement
* \brief A TemplateMeasurement class
*
* \sa MeasurementBase
*/

class TemplateMeasurement : public MeasurementBase
{
public:
    TemplateMeasurement() = default;
    ~TemplateMeasurement() = default;

    void fromString(const QString&);

    bool isValid() const override;

    QString toString() const override;

    static TemplateMeasurement simulate();
};

Q_DECLARE_METATYPE(TemplateMeasurement);

QDebug operator<<(QDebug dbg, const TemplateMeasurement&);

#endif // TEMPLATEMEASUREMENT_H


