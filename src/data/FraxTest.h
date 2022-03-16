#ifndef FRAXTEST_H
#define FRAXTEST_H

#include "TestBase.h"
#include "FraxMeasurement.h"

class FraxTest : public TestBase<FraxMeasurement>
{
public:
    FraxTest();
    ~FraxTest() = default;

    void fromFile(const QString &);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    bool isValid() const override;

    // String keys are converted to snake_case
    //
    QJsonObject toJsonObject() const override;

    void simulate(const QJsonObject&);

private:

    QList<QString> m_outputKeyList;

};

Q_DECLARE_METATYPE(FraxTest);

#endif // FRAXTEST_H
