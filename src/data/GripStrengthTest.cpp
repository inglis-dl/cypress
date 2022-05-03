#include "GripStrengthTest.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

const q_stringMap GripStrengthTest::testMetaMap = {
    {"ExamID","exam_id"},
    {"TestID","test_id"},
    {"Rung","rung"},
    {"MaxReps","max_reps"},
    {"Sequence","sequence"},
    {"RestTime","rest_time"},
    {"Threshold","threshold"},
    {"Units","units"},
};

// the minimum output data keys required from a successful a test
//
GripStrengthTest::GripStrengthTest()
{
    m_outputKeyList << "patient_id";
    m_outputKeyList.append(testMetaMap.values());
}

void GripStrengthTest::fromParradox(const QString& gripTestPath, const QString& gripTestDataPath)
{
    int participantId = 16777216;
    // Read in test information
    ParadoxReader gripTestReader(gripTestPath);
    q_paradoxBlocks testBlocks = gripTestReader.Read();
    foreach(const auto block, testBlocks) {
        if (block.count() == 1){
            QJsonObject record = block[0];
            qDebug() << "TestId: " << record["TestId"].toInt();
            if (record["TestID"].toInt() == participantId && record["Test"].toString().contains("Five Position Grip")) {
                qDebug() << "Worked";
                foreach(const auto tag, testMetaMap.toStdMap()){
                    if (record.contains(tag.first)) {
                        addMetaData(tag.second, record[tag.first].toVariant());
                    }
                }
            }
        }
    }

    // TODO: figure out what the expected measurement count is
    // For now set way higher than expected so measurements arent lost
    setExpectedMeasurementCount(1000);

    //Read in measurements
    ParadoxReader gripTestDataReader(gripTestDataPath);
    q_paradoxBlocks testDataBlocks = gripTestDataReader.Read();
    foreach(const auto block, testDataBlocks) {
        foreach(const auto record, block) {
            if (record["TestID"].toInt() == participantId) {
                GripStrengthMeasurement measurement;
                measurement.fromRecord(&record);
                if (measurement.isValid()) {
                    addMeasurement(measurement);
                }
            }
        }
    }
}

// String representation for debug and GUI display purposes
//
QString GripStrengthTest::toString() const
{
    QString str;
    if(isValid())
    {
      QStringList list;
      foreach(auto measurement, m_measurementList)
      {
        list << measurement.toString();
      }
      str = list.join("\n");
    }
    return str;
}

bool GripStrengthTest::isValid() const
{
    bool okMeta = true;
    foreach(auto key, m_outputKeyList)
    {
      if(!hasMetaData(key))
      {
         okMeta = false;
         break;
       }
    }
    bool okTest = getMeasurementCount() == getExpectedMeasurementCount();
    if(okTest)
    {
      foreach(auto m, m_measurementList)
      {
        if(!m.isValid())
        {
          okTest = false;
          break;
        }
      }
    }
    return okMeta && okTest;
}

QJsonObject GripStrengthTest::toJsonObject() const
{
    QJsonArray jsonArr;
    foreach(auto m, m_measurementList)
    {
      jsonArr.append(m.toJsonObject());
    }
    QJsonObject json;
    if(!metaDataIsEmpty())
    {
      json.insert("test_meta_data",m_metaData.toJsonObject());
    }
    json.insert("test_results",jsonArr);
    return json;
}
