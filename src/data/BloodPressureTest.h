#ifndef BLOODPRESSURETEST_H
#define BLOODPRESSURETEST_H

#include "TestBase.h"
#include "BloodPressureMeasurement.h"

#include <QList>
#include <QString>
#include <QJsonObject>

class BloodPressureTest : public TestBase<BloodPressureMeasurement>
{
public:
    BloodPressureTest();
    ~BloodPressureTest() = default;

    void addMeasurement(const int& sbp, const int& dbp, const int& pulse,
                        const QDateTime& start, const QDateTime& end,
                        const int& readingNum);
    void addAverageMeasurement(const int& sbpAvg, const int& dbpAvg, const int& pulseAvg);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    bool isValid() const override;

    // String keys are converted to snake_case
    //
    QJsonObject toJsonObject() const override;

    bool verifyReviewData(const int sbp, const int dbp, const int pulse);

    QString firstMeasurementToString() const;
    QString avgMeasurementToString() const;
    QString allAvgMeasurementToString() const;

    void setArmBandSize(const QString& size) { addMetaDataCharacteristic("arm band size", size); }
    void setArm(const QString& arm) { addMetaDataCharacteristic("arm used", arm); }
    bool armInformationSet() const;

private:
    QList<QString> m_outputKeyList;
    void storeAllAverageMetaData(int sbpTotal, int dbpTotal, int pulseTotal);
    bool hasFirstMeasurementData() const;
    bool hasAvgMeasurementData() const;
    bool hasAllAvgMeasurementData() const;
};

Q_DECLARE_METATYPE(BloodPressureTest);

#endif // BLOODPRESSURETEST_H

