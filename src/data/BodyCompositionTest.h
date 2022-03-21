#ifndef BODYCOMPOSITIONTEST_H
#define BODYCOMPOSITIONTEST_H

#include "TestBase.h"
#include "BodyCompositionMeasurement.h"

class BodyCompositionTest : public TestBase<BodyCompositionMeasurement>
{
public:
    BodyCompositionTest();
    ~BodyCompositionTest() = default;

    void fromArray(const QByteArray &);

    void simulate(const double&, const QString&, double);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    QStringList toStringList() const;

    bool isValid() const override;

    // String keys are converted to snake_case
    //
    QJsonObject toJsonObject() const override;

private:
    QList<QString> m_outputKeyList;
    QByteArray m_array;
    QString readArray(const quint8 &, const quint8 &) const;

    QString  readBodyType() const;
    QString  readGender() const;
    QVariant readHeight() const;
    QVariant readWeight() const;
    QVariant readImpedance() const;
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
