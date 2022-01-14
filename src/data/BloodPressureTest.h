#ifndef BLOODPRESSURETEST_H
#define BLOODPRESSURETEST_H

#include "TestBase.h"
#include "BloodPressureMeasurement.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>

class BloodPressureTest : public TestBase<BloodPressureMeasurement>
{
public:
    BloodPressureTest();
    ~BloodPressureTest() = default;

    void fromFile(const QString&);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    bool isValid() const override;

    // String keys are converted to snake_case
    //
    QJsonObject toJsonObject() const override;

private:
    QList<QString> m_outputKeyList;
};

Q_DECLARE_METATYPE(BloodPressureTest);

#endif // BLOODPRESSURETEST_H

