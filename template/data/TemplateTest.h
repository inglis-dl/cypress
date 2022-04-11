#ifndef TEMPLATETEST_H
#define TEMPLATETEST_H

#include "../../src/data/TestBase.h"
#include "TemplateMeasurement.h"

class TemplateTest : public TestBase<TemplateMeasurement>
{
public:
    TemplateTest();
    ~TemplateTest() = default;

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

Q_DECLARE_METATYPE(TemplateTest);

#endif // TEMPLATETEST_H

