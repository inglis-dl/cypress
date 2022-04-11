#ifndef TONOMETERTEST_H
#define TONOMETERTEST_H

#include "TestBase.h"
#include "TonometerMeasurement.h"

QT_FORWARD_DECLARE_CLASS(QJsonArray)

class TonometerTest : public TestBase<TonometerMeasurement>
{
public:
    TonometerTest();
    ~TonometerTest() = default;

    void fromVariant(const QVariant&);

    void simulate(const QVariantMap&);

    // String representation for debug purposes
    //
    QString toString() const override;

    // Get the measurements by side for UI display
    //
    QStringList getMeasurementStrings(const QString&) const;

    bool isValid() const override;

    // String keys are converted to snake_case
    //
    QJsonObject toJsonObject() const override;

    static const q_stringMap metaLUT;
    static q_stringMap initMetaLUT();

private:
    QStringList m_outputKeyList;
};

Q_DECLARE_METATYPE(TonometerTest);

#endif // TONOMETERTEST_H
