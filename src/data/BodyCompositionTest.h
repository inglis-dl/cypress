#ifndef BODYCOMPOSITIONTEST_H
#define BODYCOMPOSITIONTEST_H

#include "TestBase.h"
#include "BodyCompositionMeasurement.h"

class BodyCompositionTest : public TestBase<BodyCompositionMeasurement>
{
public:
    BodyCompositionTest() = default;
    ~BodyCompositionTest() = default;

    void fromArray(const QByteArray &);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    bool isValid() const override;

    // String keys are converted to snake_case
    //
    QJsonObject toJsonObject() const override;

private:
    QByteArray m_array;
    QString readArray(const quint8 &, const quint8 &) const;

    QString  readBodyType() const;
    QString  readGender() const;
    QVariant readHeight() const;
    QVariant readWeight() const;
    QVariant readImpedence() const;
    QVariant readFatPercent() const;
    QVariant readFatMass() const;
    QVariant readFatFreeMass() const;
    QVariant readTotalBodyWater() const;
    QVariant readAge() const;
    QVariant readBMI() const;
    QVariant readBMR() const;
};

Q_DECLARE_METATYPE(BodyCompositionTest);

#endif // BODYCOMPOSITIONTEST_H
