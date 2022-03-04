#ifndef TRUEFLOWSPIROMETERTEST_H
#define TRUEFLOWSPIROMETERTEST_H

#include "../../src/data/TestBase.h"
#include "TrueFlowSpirometerMeasurement.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>

class TrueFlowSpirometerTest : public TestBase<TrueFlowSpirometerMeasurement>
{
public:
    TrueFlowSpirometerTest();
    ~TrueFlowSpirometerTest() = default;

    bool loadData();

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

Q_DECLARE_METATYPE(TrueFlowSpirometerTest);

#endif // TRUEFLOWSPIROMETERTEST_H

