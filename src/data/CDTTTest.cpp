#include "CDTTTest.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>

// the minimum output data keys required from a successful a test
//
CDTTTest::CDTTTest()
{
    m_outputKeyList << "user id";
    m_outputKeyList << "datetime";
    m_outputKeyList << "language";
    m_outputKeyList << "talker";
    m_outputKeyList << "mode";
    m_outputKeyList << "digits";
    m_outputKeyList << "list number";
    m_outputKeyList << "msk signal";
    m_outputKeyList << "test ear";
    m_outputKeyList << "sp level";
    m_outputKeyList << "msk level";
    m_outputKeyList << "number of measurements";

    // These show up depending on mode
    // TODO: figure out which mode CLSA uses and only keep the relevant ones
    /**
      m_outputKeyList << "adaptive srt";
      m_outputKeyList << "adaptive st dev";
      m_outputKeyList << "adaptive reversals";
      m_outputKeyList << "triplets score";
      m_outputKeyList << "triplets percent";
      m_outputKeyList << "digit 1 score";
      m_outputKeyList << "digit 1 percent";
      m_outputKeyList << "digit 2 score";
      m_outputKeyList << "digit 2 percent";
      m_outputKeyList << "digit 3 score";
      m_outputKeyList << "digit 3 percent";
    */
}

//TODO: implement void CDTTTest::simulate()
//

void CDTTTest::fromFile(const QString &fileName)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "xlsx_connection");
    if(QFileInfo::exists(fileName))
    {
        qDebug() << "OK, reading input file " << fileName;

        //TODO: impl for linux or insert ifdef OS blockers
        //
        db.setDatabaseName("DRIVER={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};DBQ=" + fileName);
        if(db.open())
        {
            reset();
            if (queryTestMetaData(db))
            {
              queryTestMeasurements(db);
              int n = getNumberOfMeasurements();
              addMetaDataCharacteristic("number of measurements", n);
            }
        }
    }
}

// String representation for debug and GUI display purposes
//
QString CDTTTest::toString() const
{
    QString s;
    if(isValid())
    {
        QStringList l;
        for(auto&& x : m_measurementList)
        {
            l << x.toString();
        }
        s = l.join("\n");
    }
    return s;
}

bool CDTTTest::isValid() const
{
    bool okMeta = true;
    for(auto&& key : m_outputKeyList)
    {
      if(!hasMetaDataCharacteristic(key))
      {
         okMeta = false;
         qDebug() << "ERROR: missing test meta data " << key;
         break;
       }
    }
    bool okTest = 0 < getNumberOfMeasurements();
    if(okTest)
    {
      for(auto&& x : m_measurementList)
      {
        if(!x.isValid())
        {
          okTest = false;
          qDebug() << "ERROR: invalid test measurement";
          break;
        }
      }
    }

    return okMeta && okTest;
}

// String keys are converted to snake_case
//
QJsonObject CDTTTest::toJsonObject() const
{
    QJsonArray jsonArr;
    for(auto&& x : m_measurementList)
    {
        jsonArr.append(x.toJsonObject());
    }
    QJsonObject json;
    json.insert("test_meta_data",m_metaData.toJsonObject());
    json.insert("test_results",jsonArr);
    return json;
}

bool CDTTTest::queryTestMetaData(const QSqlDatabase &db)
{
    QString idQueryString = QString("select * from [%0$A1:B1]").arg("Main");
    QSqlQuery idQuery(idQueryString, db);
    QSqlRecord idRec = idQuery.record();
    qDebug() << "Id Fields: " << idRec.fieldName(0) + "," + idRec.fieldName(1);
    if("Subject ID:" == idRec.fieldName(0))
    {
        addMetaDataCharacteristic("user id", idRec.fieldName(1).toInt());
    }

    QString headerQueryString = QString("select * from [%0$A4:U6]").arg("Main");
    QSqlQuery headerQuery(headerQueryString, db);
    QSqlRecord headerRec = headerQuery.record();
    if(!headerValid(headerRec))
    {
        return false;
    }

    headerQuery.next();
    loadHeaderMetaData(headerQuery);

    if(headerQuery.next())
    {
        // more than 1 test present
        // TODO: Figure out what to do in this case. Should the newest test be used?
        //       Should there be some sort of date check
        qDebug() << "############### WARNING: Multiple tests detected ###############";
    }

    return true;
}

bool CDTTTest::headerValid(const QSqlRecord &record) const
{
    QString debugString = record.fieldName(0);
    for(int i = 1; i < 21; i++)
    {
        debugString += "," + record.fieldName(i);
    }
    qDebug() << debugString;

    bool dateValid = record.fieldName(0) == "Date & time";
    bool langValid = record.fieldName(1) == "Language";
    bool talkerValid = record.fieldName(2) == "Talker";
    bool modeValid = record.fieldName(3) == "Mode";
    bool digValid = record.fieldName(4) == "Digits";
    bool listValid = record.fieldName(5) == "List #";
    bool mskSignalValid = record.fieldName(6) == "MSK signal";
    bool earValid = record.fieldName(7) == "Test Ear";
    bool splValid = record.fieldName(8) == "SP level";
    bool mskLevelValid = record.fieldName(9) == "MSK level";
    bool srtValid = record.fieldName(10) == "SRT";
    bool stValid = record.fieldName(11).contains("St") && record.fieldName(11).contains("Dev");
    bool revValid = record.fieldName(12) == "Reversals";
    bool score1Valid = record.fieldName(13).contains("Score");
    bool percent1Valid = record.fieldName(14).contains("%");
    bool score2Valid = record.fieldName(15).contains("Score");
    bool percent2Valid = record.fieldName(16).contains("%");
    bool score3Valid = record.fieldName(17).contains("Score");
    bool percent3Valid = record.fieldName(18).contains("%");
    bool score4Valid = record.fieldName(19).contains("Score");
    bool percent4Valid = record.fieldName(20).contains("%");

    bool valid = dateValid && langValid && talkerValid && modeValid && digValid && listValid && mskSignalValid && earValid
        && splValid && mskLevelValid && srtValid && stValid && revValid && score1Valid && score2Valid && score3Valid && score4Valid
        && percent1Valid && percent2Valid && percent3Valid && percent4Valid;
    qDebug() << "Header valid: " << valid;
    return valid;
}

void CDTTTest::loadHeaderMetaData(const QSqlQuery &query)
{
    // NOTE: At this point the header has been validated to ensure 
    //       the data at each index contains the expected data
    addMetaIfDataExists(query, "datetime", 0);
    addMetaIfDataExists(query, "language", 1);
    addMetaIfDataExists(query, "talker", 2);
    addMetaIfDataExists(query, "mode", 3);
    addMetaIfDataExists(query, "digits", 4);
    addMetaIfDataExists(query, "list number", 5);
    addMetaIfDataExists(query, "msk signal", 6);
    addMetaIfDataExists(query, "test ear", 7);
    addMetaIfDataExists(query, "sp level", 8);
    addMetaIfDataExists(query, "msk level", 9);
    addMetaIfDataExists(query, "adaptive srt", 10);
    addMetaIfDataExists(query, "adaptive st dev", 11);
    addMetaIfDataExists(query, "adaptive reversals", 12);
    addMetaIfDataExists(query, "triplets score", 13);
    addMetaIfDataExists(query, "triplets percent", 14);
    addMetaIfDataExists(query, "digit 1 score", 15);
    addMetaIfDataExists(query, "digit 1 percent", 16);
    addMetaIfDataExists(query, "digit 2 score", 17);
    addMetaIfDataExists(query, "digit 2 percent", 18);
    addMetaIfDataExists(query, "digit 3 score", 19);
    addMetaIfDataExists(query, "digit 3 percent", 20);
}

void CDTTTest::addMetaIfDataExists(const QSqlQuery &query, const QString &metaName, const int &loc)
{
    QVariant val = query.value(loc);
    qDebug() << metaName << ": " << val;
    if(!val.isNull() && "" != val)
    {
        addMetaDataCharacteristic(metaName, val);
        qDebug() << metaName << " meta data added";
    }
}

bool CDTTTest::queryTestMeasurements(const QSqlDatabase &db)
{
    QString pageName = QString("%0-%1").arg(getMetaDataCharacteristic("language").toString(), getMetaDataCharacteristic("talker").toString());
    if(measurementHeaderMetaDataMatches(db, pageName))
    {
        return queryMeasurementsSection(db, pageName, 6);
    }
    // TODO: Decide what to do in this case
    return false;
}

bool CDTTTest::measurementHeaderMetaDataMatches(const QSqlDatabase &db, const QString &pageName) const
{
    QString queryString = QString("select * from [%0$A1:G4]").arg(pageName);
    QSqlQuery query(queryString, db);
    QSqlRecord rec = query.record();
    qDebug() << "Fields: " << rec.fieldName(0) + "," + rec.fieldName(1) + "," + rec.fieldName(2) + ","
        + rec.fieldName(3) + "," + rec.fieldName(4) + "," + rec.fieldName(5) + "," + rec.fieldName(6);
    bool languageMatches = measurementLanguageMatches(rec);
    bool numTestsMatches = measurementNumTestsMatches(rec);
    query.next();
    bool talkerMatches = measurementTalkerMatches(query);
    query.next();
    bool maskerMatches = measurementMaskerMatches(query);
    query.next();
    bool dateMatches = measurementDateMatches(query);
    qDebug() << QString("Header (Language: %0 Num tests: %1 Talker: %2 Masker: %3 Date: %4")
        .arg(languageMatches ? "True" : "False", numTestsMatches ? "True" : "False", talkerMatches ? "True" : "False", 
            maskerMatches ? "True" : "False", dateMatches ? "True" : "False");
    return languageMatches && numTestsMatches && talkerMatches && maskerMatches && dateMatches;
}

bool CDTTTest::measurementLanguageMatches(const QSqlRecord &record) const
{
    QString recLanguage = record.fieldName(2);
    QString metaLanguage = getMetaDataCharacteristic("language").toString();
    bool languageMatches = recLanguage == metaLanguage;
    qDebug() << QString("Language (Measurement: %0 Meta: %1 Equal: %2)").arg(recLanguage, metaLanguage, languageMatches ? "True" : "False");
    return languageMatches;
}

bool CDTTTest::measurementNumTestsMatches(const QSqlRecord &record) const
{
    int numTests = record.fieldName(6).toInt();
    qDebug() << QString("Num Tests: %0").arg(numTests);
    if(1 == numTests)
    {
        return true;
    }
    else if (1 < numTests)
    {
        // TODO: Figure out what to do in this case
        return false;
    }
    else
    {
        return false;
    }
}

bool CDTTTest::measurementTalkerMatches(const QSqlQuery &query) const
{
    QString measeurementTalker = query.value(2).toString();
    QString metaTalker = getMetaDataCharacteristic("talker").toString();
    bool talkerMatches = measeurementTalker == metaTalker;
    qDebug() << QString("Talker (Measurement: %0, Meta: %1 Equal: %2)").arg(measeurementTalker, metaTalker, talkerMatches ? "True" : "False");
    return talkerMatches;
}

bool CDTTTest::measurementMaskerMatches(const QSqlQuery &query) const
{
    float measeurementMasker = query.value(2).toFloat();
    float metaMasker = getMetaDataCharacteristic("msk level").toFloat();
    bool maskerMatches = measeurementMasker == metaMasker;
    qDebug() << QString("Masker (Measurement: %0, Meta: %1 Equal: %2)")
        .arg(QString::number(measeurementMasker), QString::number(metaMasker),maskerMatches ? "True" : "False");
    return maskerMatches;
}

bool CDTTTest::measurementDateMatches(const QSqlQuery &query) const
{
    Q_UNUSED(query)
    // TODO: Figure out how to convert from qvariant to date and datetime then compare just date portion
    // return query.value(2).toString() == getMetaDataCharacteristic("datetime");
    return true;
}

bool CDTTTest::queryMeasurementsSection(const QSqlDatabase &db, const QString &pageName, const int &rowStart)
{
    QString queryString = QString("select * from [%0$A%1:G%2]").arg(pageName, QString::number(rowStart), QString::number(rowStart + 30));
    QSqlQuery query(queryString, db);
    QSqlRecord rec = query.record();
    qDebug() << "Fields: " << rec.fieldName(0) + "," + rec.fieldName(1) + "," + rec.fieldName(2) + "," + rec.fieldName(3);
    bool listNumMatches = measurementListNumMatches(rec);
    query.next();
    query.next();
    bool speechLevelMatches = measurementSpeechLevelMatches(query);
    query.next();
    bool modeMatches = measurementModeMatches(query);
    query.next();
    query.next();
    bool headerCorrect = measurementHeaderMatches(query);
    if(listNumMatches && speechLevelMatches && modeMatches && headerCorrect)
    {
        bool measurementsCollected = collectMeasurements(query);
        return measurementsCollected;
    }
    else
    {
        qDebug() << "Incorrect formatting of page " << pageName;
        return false;
    }  
}

bool CDTTTest::measurementListNumMatches(const QSqlRecord &record) const
{
    int measureListNum = record.fieldName(2).toInt();
    int metaListNum = getMetaDataCharacteristic("list number").toInt();
    bool listNumMatches = measureListNum == metaListNum;
    qDebug() << QString("List Num (Measurement: %0 Meta: %1 Matches: %2")
            .arg(QString::number(measureListNum) , QString::number(metaListNum), listNumMatches ? "True" : "False");
    return listNumMatches;
}

bool CDTTTest::measurementSpeechLevelMatches(const QSqlQuery &query) const
{
    float measureSpeechLevel = query.value(2).toFloat();
    float metaSpeechLevel = getMetaDataCharacteristic("sp level").toFloat();
    bool speechLevelMatches = measureSpeechLevel == metaSpeechLevel;
    qDebug() << QString("Speech Level (Measurement: %0 Meta: %1 Matches: %2")
        .arg(QString::number(measureSpeechLevel), QString::number(metaSpeechLevel), speechLevelMatches ? "True" : "False");
    return speechLevelMatches;
}

bool CDTTTest::measurementModeMatches(const QSqlQuery &query) const
{
    QString measureMode = query.value(4).toString();
    QString metaMode = getMetaDataCharacteristic("mode").toString();
    bool modeMatches = measureMode == metaMode;
    qDebug() << QString("Mode (Measurement: %0 Meta: %1 Matches: %2")
        .arg(measureMode, metaMode, modeMatches ? "True" : "False");
    return modeMatches;
}

bool CDTTTest::measurementHeaderMatches(QSqlQuery query) const
{
    QString queryStimulus = query.value(1).toString();
    QString queryResponse = query.value(4).toString();
    bool line1Correct = queryStimulus == "STIMULUS" && queryResponse == "RESPONSE";
    qDebug() << QString("Line 1 Header : %0,%1,%2,%3,%4,%5")
        .arg(query.value(0).toString(), query.value(1).toString(), query.value(2).toString(),
            query.value(3).toString(), query.value(4).toString(), query.value(5).toString());
    qDebug() << QString("Line 1 Correct: %0 Expected: %1,%2 Actual: %3,%4")
        .arg(line1Correct ? "True" : "False", "STIMULUS", "RESPONSE", queryStimulus, queryResponse);
    query.next();

    QString queryDigits1 = query.value(1).toString();
    QString queryDigits2 = query.value(4).toString();
    bool line2Correct = queryDigits1 == "DIGITS" && queryDigits2 == "DIGITS";
    qDebug() << QString("Line 2 Header : %0,%1,%2,%3,%4,%5")
        .arg(query.value(0).toString(), query.value(1).toString(), query.value(2).toString(),
            query.value(3).toString(), query.value(4).toString(), query.value(5).toString());
    qDebug() << QString("Line 2 Correct: %0 Expected: %1,%2 Actual: %3,%4")
        .arg(line2Correct ? "True" : "False", "DIGITS", "DIGITS", queryDigits1, queryDigits2);

    return line1Correct && line2Correct;
}

bool CDTTTest::collectMeasurements(QSqlQuery query)
{
    int numValidMeasurements = 0;
    while (query.next())
    {
        CDTTMeasurement newMeasurement;
        QString row(query.value(0).toString());
        for (int i = 1; i < 7; i++) {
            row += QString(",%0").arg(query.value(i).toString());
        }
        newMeasurement.fromString(row);
        if(newMeasurement.isValid())
        {
            addMeasurement(newMeasurement);
            numValidMeasurements++;
            qDebug() << newMeasurement.toString();
        }
        else
        {
            qDebug() << "Found INVALID measurement item";
        }
    }

    qDebug() << "Valid Measurements: " << numValidMeasurements;
    return numValidMeasurements > 0;
}
