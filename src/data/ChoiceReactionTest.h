#ifndef CHOICEREACTIONTEST_H
#define CHOICEREACTIONTEST_H

#include "TestBase.h"
#include "ChoiceReactionMeasurement.h"

class ChoiceReactionTest : public TestBase<ChoiceReactionMeasurement>
{
public:
    ChoiceReactionTest() = default;
    ~ChoiceReactionTest() = default;

    void fromFile(const QString &);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    bool isValid() const override;

    // String keys are converted to snake_case
    //
    QJsonObject toJsonObject() const override;

};

Q_DECLARE_METATYPE(ChoiceReactionTest);

#endif // CHOICEREACTIONTEST_H
