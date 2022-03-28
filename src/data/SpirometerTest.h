#ifndef SPIROMETERTEST_H
#define SPIROMETERTEST_H

#include "TestBase.h"
#include "SpirometerMeasurement.h"

class SpirometerTest : public TestBase<SpirometerMeasurement>
{
public:
    SpirometerTest();
    ~SpirometerTest() = default;

    bool loadData(const QString &transferOutPath, const QString& barcode);

    void fromFile(const QString&);

    void simulate(const QJsonObject&);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    QStringList getMeasurementHeaderValues() const;

    QList<QStringList> getMeasurementsAsLists() const;

    bool isValid() const override;

    // String keys are converted to snake_case
    //
    QJsonObject toJsonObject() const override;

private:
    QList<QString> m_outputKeyList;
};

Q_DECLARE_METATYPE(SpirometerTest);

#endif // SPIROMETERTEST_H

