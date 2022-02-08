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

    void fromJson(const QJsonArray&);

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

    static QMap<QString,QString> variableLUT;
    static QMap<QString,QString> metaLUT;
    static QMap<QString,QString> unitsLUT;

    static QMap<QString,QString> initVariableLUT();
    static QMap<QString,QString> initMetaLUT();
    static QMap<QString,QString> initUnitsLUT();

private:
    QList<QString> m_outputKeyList;
};

Q_DECLARE_METATYPE(TonometerTest);

#endif // TONOMETERTEST_H
