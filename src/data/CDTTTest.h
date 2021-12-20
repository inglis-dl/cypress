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

#pragma region Query Main page which contains meta data
    bool queryTestMetaData(QSqlDatabase db);
    bool headerValid(QSqlRecord record);
    void loadHeaderMetaData(QSqlQuery query);
    void addMetaIfDataExists(QSqlQuery query, QString metaName, int loc);
#pragma endregion

#pragma region Querys excel page which contains measurement data
    bool queryTestMeasurements(QSqlDatabase db);

    // Query the header of the 2nd page which contains duplicate meta data (should match main page)
    bool measurementHeaderMetaDataMatches(QSqlDatabase db, QString pageName);
    bool measurementLanguageMatches(QSqlRecord record);
    bool measurementNumTestsMatches(QSqlRecord record);
    bool measurementTalkerMatches(QSqlQuery query);
    bool measurementMaskerMatches(QSqlQuery query);
    bool measurementDateMatches(QSqlQuery query);

    // Query measuement section of 2nd page (Continas a small amount of duplicate meta data and then measurements)
    bool queryMeasurementsSection(QSqlDatabase db, QString pageName, int rowStart);
    bool measurementListNumMatches(QSqlRecord record);
    bool measurementSpeachLevelMatches(QSqlQuery query);
    bool measurementModeMatches(QSqlQuery query);
    bool measurementHeaderMatches(QSqlQuery query);
    bool collectMeasurements(QSqlQuery query);
#pragma endregion

};

Q_DECLARE_METATYPE(CDTTTest);

#endif // CDTTTEST_H
