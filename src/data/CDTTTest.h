#ifndef CDTTTEST_H
#define CDTTTEST_H

#include "TestBase.h"
#include "CDTTMeasurement.h"
#include <QJsonObject>

QT_FORWARD_DECLARE_CLASS(QSqlDatabase)

class CDTTTest : public TestBase<CDTTMeasurement>
{
public:
    CDTTTest();
    ~CDTTTest() = default;

    // read from a MS Excel file using ODBC sql
    //
    void fromDatabase(const QSqlDatabase&);

    // generate a fictitious set of results
    // arg is the interview barcode
    //
    void simulate(const QString&);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    bool isValid() const override;

    // String keys are converted to snake_case
    //
    QJsonObject toJsonObject() const override;

    QStringList toStringList() const;

private:

    QList<QString> m_outputKeyList;
    QJsonObject m_jsonObj;

    bool readBarcode(const QSqlDatabase &);
    bool readMetaData(const QSqlDatabase &);
    bool readSummary(const QSqlDatabase &);
    bool readTrialData(const QSqlDatabase &);

};

Q_DECLARE_METATYPE(CDTTTest);

#endif // CDTTTEST_H
