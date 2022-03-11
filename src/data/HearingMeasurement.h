#ifndef HEARINGMEASUREMENT_H
#define HEARINGMEASUREMENT_H

#include "Measurement.h"

/*!
 * \class HearingMeasurement
 * \brief A HearingMeasurement class
 *
 * \sa Measurement, HearingTest
 */

class HearingMeasurement : public Measurement
{   
public:
    HearingMeasurement() = default;
    ~HearingMeasurement() = default;

    HearingMeasurement(const QString &, const int &, const QString &);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    bool isValid() const override;

    static QMap<QString,QString> initCodeLookup();
    static QMap<QString,QString> initOutcomeLookup();
    static QMap<int,QString> initFrequencyLookup();

    friend class HearingTest;

    void fromCode(const QString &, const int &, const QString &);

private:
    static const QMap<QString,QString> codeLookup;
    static const QMap<QString,QString> outcomeLookup;
    static const QMap<int,QString> frequencyLookup;

};

Q_DECLARE_METATYPE(HearingMeasurement);

QDebug operator<<(QDebug dbg, const HearingMeasurement&);

#endif // HEARINGMEASUREMENT_H
