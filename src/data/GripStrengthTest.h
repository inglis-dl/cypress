#ifndef GRIPSTRENGTHTEST_H
#define GRIPSTRENGTHTEST_H

#include "../../src/data/TestBase.h"
#include "GripStrengthMeasurement.h"

class GripStrengthTest : public TestBase<GripStrengthMeasurement>
{
public:
    GripStrengthTest();
    ~GripStrengthTest() = default;

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    bool isValid() const override;

    // String keys are converted to snake_case
    //
    QJsonObject toJsonObject() const override;

private:
    QStringList m_outputKeyList;
};

Q_DECLARE_METATYPE(GripStrengthTest);

#endif // GRIPSTRENGTHTEST_H

