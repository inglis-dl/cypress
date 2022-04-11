#ifndef SPIROMETERMEASUREMENT_H
#define SPIROMETERMEASUREMENT_H

#include "Measurement.h"

/*!
* \class SpirometerMeasurement
* \brief A SpirometerMeasurement class
*
* \sa Measurement
*/

typedef QMap<QString,QString> q_stringMap;

class SpirometerMeasurement : public Measurement
{
public:

    enum ResultType {
        typeUnknown,
        typeBestValues,
        typeTrial
    };

    void setResultType(const ResultType& type){m_type = type;};
    ResultType getResultType(){return m_type;}

    SpirometerMeasurement() = default;
    ~SpirometerMeasurement() = default;

    bool isValid() const override;

    QString toString() const override;

    void simulate();

    static const QStringList parameterList;
    static const q_stringMap channelMap;
    static const q_stringMap resultMap;
    static const q_stringMap trialMap;

private:

    ResultType m_type { typeUnknown };

};

Q_DECLARE_METATYPE(SpirometerMeasurement);

QDebug operator<<(QDebug dbg, const SpirometerMeasurement&);

#endif // SPIROMETERMEASUREMENT_H


