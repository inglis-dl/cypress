#include "FraxTest.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>


/**
 * sample contents of output.txt from blackbox.exe
 *
 * t,19,50.2,0,21.60494,1,0,1,0,0,0, 0, -1.1, 6.37,1.17,4.47,0.34
 *
 * 0 1  2  3 4        5 6 7 8 9 10 11 12 13   14   15   16
 * 0 : t or z (score input for field at position 12)
 * 1 : country code (19 = Canada)
 * 2 : Age in years (positive real number, can have decimals) Age should be 40-90.
 * 3 : Sex (0 : men, 1 : women)
 * 4 : BMI kg/m2 (positive real number, can have decimals)
 * 5 : Previous fracture (0 : no, 1 : yes)
 * 6 : Parental history of hip fracture (0 : no, 1 : yes)
 * 7 : Current smoker (0 : no, 1 : yes)
 * 8 : Gluccocorticoid (0 : no, 1 : yes)
 * 9 : Rheumatoid Arthritis (0 : no, 1 : yes)
 * 10: Secondary osteoporosis (0 : no, 1 : yes)
 * 11: Alcohol more than two drinks a day (0 : no, 1 : yes)
 * 12: Femoral neck BMD (real number, can have decimals). There are two inputs possible, T-score or Z-score
 * 13: 10 year probability (x 100) of osteoporotic fracture, calculated without knowing BMD (positive real number with decimals)
 * 14: 10 year probability (x 100) of hip fracture, calculated without knowing BMD (positive real number with decimals)
 * 15: 10 year probability (x 100) of osteoporotic fracture, calculated knowing BMD (real number with decimals)
 * 16: 10 year probability (x 100) of hip fracture, calculated knowing BMD (real number with decimals)
 *
 */

FraxTest::FraxTest()
{
    m_outputKeyList << "type";
    m_outputKeyList << "country_code";
    m_outputKeyList << "age";
    m_outputKeyList << "sex";
    m_outputKeyList << "bmi";
    m_outputKeyList << "previous_fracture";
    m_outputKeyList << "parent_hip_fracture";
    m_outputKeyList << "current_smoker";
    m_outputKeyList << "gluccocorticoid";
    m_outputKeyList << "rheumatoid_arthritis";
    m_outputKeyList << "secondary_osteoporosis";
    m_outputKeyList << "alcohol";
    m_outputKeyList << "femoral_neck_bmd";
}

void FraxTest::fromFile(const QString &fileName)
{
    QFile ifile(fileName);
    if(ifile.open(QIODevice::ReadOnly))
    {
        qDebug() << "OK, reading input file " << fileName;

        QTextStream instream(&ifile);
        QString line = instream.readLine();
        if(false == instream.atEnd())
        {
            qDebug() << "Frax: More lines of content than expected";
        }
        ifile.close();        
        reset();

        QStringList list = line.split(",");
        if(17 == list.size())
        {
           FraxMeasurement m;
           m.setCharacteristic("type","osteoporotic fracture");
           m.setCharacteristic("probability", list.at(13).toDouble());
           m.setCharacteristic("units","%");
           addMeasurement(m);
           m.setCharacteristic("type","hip fracture");
           m.setCharacteristic("probability", list.at(14).toDouble());
           addMeasurement(m);
           m.setCharacteristic("type","osteoporotic fracture bmd");
           m.setCharacteristic("probability", list.at(15).toDouble());
           addMeasurement(m);
           m.setCharacteristic("type","hip fracture bmd");
           m.setCharacteristic("probability", list.at(16).toDouble());
           addMeasurement(m);

           addMetaDataCharacteristic("type",list.at(0).toLower());
           addMetaDataCharacteristic("country_code",list.at(1).toUInt());
           addMetaDataCharacteristic("age",list.at(2).toDouble());
           addMetaDataCharacteristic("sex",list.at(3).toUInt());
           addMetaDataCharacteristic("bmi",list.at(4).toDouble());
           addMetaDataCharacteristic("previous_fracture",list.at(5).toUInt());
           addMetaDataCharacteristic("parent_hip_fracture",list.at(6).toUInt());
           addMetaDataCharacteristic("current_smoker",list.at(7).toUInt());
           addMetaDataCharacteristic("gluccocorticoid",list.at(8).toUInt());
           addMetaDataCharacteristic("rheumatoid_arthritis",list.at(9).toUInt());
           addMetaDataCharacteristic("secondary_osteoporosis",list.at(10).toUInt());
           addMetaDataCharacteristic("alcohol",list.at(11).toUInt());
           addMetaDataCharacteristic("femoral_neck_bmd",list.at(12).toDouble());
        }
    }
}

// String representation for debug and GUI display purposes
//
QString FraxTest::toString() const
{
    QString outStr;
    if (isValid())
    {
        QStringList tempList;
        for (auto&& measurement : m_measurementList)
        {
            tempList << measurement.toString();
        }
        outStr = tempList.join("\n");
    }
    return outStr;
}

bool FraxTest::isValid() const
{
    bool okMeta = true;
    for(auto&& x : m_outputKeyList)
    {
      if(!hasMetaDataCharacteristic(x))
      {
         okMeta = false;
         qDebug() << "ERROR: missing test meta data " << x;
         break;
       }
    }
    bool okTest = 4 == getNumberOfMeasurements();
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

QJsonObject FraxTest::toJsonObject() const
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
