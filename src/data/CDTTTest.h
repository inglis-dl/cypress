#ifndef CDTTTEST_H
#define CDTTTEST_H

#include "TestBase.h"
#include "CDTTMeasurement.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>

class CDTTTest : public TestBase<CDTTMeasurement>
{
public:
    CDTTTest();
    ~CDTTTest() = default;

    void fromFile(const QString &);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    bool isValid() const override;

    // String keys are converted to snake_case
    //
    QJsonObject toJsonObject() const override;

private:
    QList<QString> m_outputKeyList;

    bool queryTestMetaData(const QSqlDatabase &);
    bool headerValid(const QSqlRecord &) const;
    void loadHeaderMetaData(const QSqlQuery &);
    void addMetaIfDataExists(const QSqlQuery &, const QString &, const int &);

    bool queryTestMeasurements(const QSqlDatabase &);

    // Query the header of the 2nd page which contains duplicate meta data (should match main page)
    bool measurementHeaderMetaDataMatches(const QSqlDatabase &, const QString &) const;
    bool measurementLanguageMatches(const QSqlRecord &) const;
    bool measurementNumTestsMatches(const QSqlRecord &) const;
    bool measurementTalkerMatches(const QSqlQuery &) const;
    bool measurementMaskerMatches(const QSqlQuery &) const;
    bool measurementDateMatches(const QSqlQuery &) const;

    // Query measuement section of 2nd page (Continas a small amount of duplicate meta data and then measurements)
    bool queryMeasurementsSection(const QSqlDatabase&, const QString&, const int &);
    bool measurementListNumMatches(const QSqlRecord&) const;
    bool measurementSpeechLevelMatches(const QSqlQuery&) const;
    bool measurementModeMatches(const QSqlQuery&) const;
    bool measurementHeaderMatches(QSqlQuery query) const;
    bool collectMeasurements(QSqlQuery query);

};

Q_DECLARE_METATYPE(CDTTTest);

#endif // CDTTTEST_H
