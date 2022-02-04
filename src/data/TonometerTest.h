#ifndef TONOMETERTEST_H
#define TONOMETERTEST_H

#include "TestBase.h"
#include "TonometerMeasurement.h"

QT_FORWARD_DECLARE_CLASS(QSqlQuery)

class TonometerTest : public TestBase<TonometerMeasurement>
{
public:
    TonometerTest();
    ~TonometerTest() = default;

    void fromQuery(QSqlQuery*);

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

Q_DECLARE_METATYPE(TonometerTest);

#endif // TONOMETERTEST_H
