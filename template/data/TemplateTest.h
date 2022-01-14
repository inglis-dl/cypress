#ifndef TEMPLATETEST_H
#define TEMPLATETEST_H

#include "../../src/data/TestBase.h"
#include "TemplateMeasurement.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>

class TemplateTest : public TestBase<TemplateMeasurement>
{
public:
    TemplateTest();
    ~TemplateTest() = default;

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

Q_DECLARE_METATYPE(TemplateTest);

#endif // TEMPLATETEST_H

