#ifndef GRIPSTRENGTHMEASUREMENT_H
#define GRIPSTRENGTHMEASUREMENT_H

#include "../../src/data/Measurement.h"

/*!
* \class GripStrengthMeasurement
* \brief A GripStrengthMeasurement class
*
* \sa Measurement
*/

typedef QMap<QString, QString> q_stringMap;

class GripStrengthMeasurement : public Measurement
{
public:
    GripStrengthMeasurement() = default;
    ~GripStrengthMeasurement() = default;

    void fromRecord(const QJsonObject* record);

    bool isValid() const override;

    QString toString() const override;

    static GripStrengthMeasurement simulate();
    static const q_stringMap trialMap;
};

Q_DECLARE_METATYPE(GripStrengthMeasurement);

QDebug operator<<(QDebug dbg, const GripStrengthMeasurement&);

#endif // GRIPSTRENGTHMEASUREMENT_H


