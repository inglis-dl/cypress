#ifndef BODYCOMPOSITIONANALYZERTEST_H
#define BODYCOMPOSITIONANALYZERTEST_H

#include "TestBase.h"
#include "BodyCompositionMeasurement.h"

class BodyCompositionAnalyzerTest : public TestBase<BodyCompositionMeasurement>
{
public:
    BodyCompositionAnalyzerTest() = default;
    ~BodyCompositionAnalyzerTest() = default;

    void fromArray(const QByteArray &);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    bool isValid() const override;

    // String keys are converted to snake_case
    //
    QJsonObject toJsonObject() const override;

    static bool hasEndCode(const QByteArray &);

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

Q_DECLARE_METATYPE(BodyCompositionAnalyzerTest);

#endif // BODYCOMPOSITIONANALYZERTEST_H
