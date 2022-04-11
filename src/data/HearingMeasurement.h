#ifndef HEARINGMEASUREMENT_H
#define HEARINGMEASUREMENT_H

#include "Measurement.h"

/*!
 * \class HearingMeasurement
 * \brief A HearingMeasurement class
 *
 * \sa Measurement, HearingTest
 */

typedef QMap<QString,QString> q_stringMap;

class HearingMeasurement : public Measurement
{   
public:
    HearingMeasurement() = default;
    ~HearingMeasurement() = default;

    HearingMeasurement(const QString&, const int&, const QString&);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    bool isValid() const override;

    static q_stringMap initCodeLookup();
    static q_stringMap initOutcomeLookup();
    static QMap<int,QString> initFrequencyLookup();

    friend class HearingTest;

    void fromCode(const QString&, const int&, const QString&);

private:
    static const q_stringMap codeLookup;
    static const q_stringMap outcomeLookup;
    static const QMap<int,QString> frequencyLookup;

};

Q_DECLARE_METATYPE(HearingMeasurement);

QDebug operator<<(QDebug dbg, const HearingMeasurement&);

#endif // HEARINGMEASUREMENT_H
