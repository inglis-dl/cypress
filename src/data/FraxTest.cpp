#include "FraxTest.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include "../auxiliary/Utilities.h"

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
           m.setAttribute("type","osteoporotic_fracture");
           m.setAttribute("probability", Measurement::Value(list.at(13).toDouble(),"%"));
           addMeasurement(m);
           m.setAttribute("type","hip_fracture");
           m.setAttribute("probability", Measurement::Value(list.at(14).toDouble(),"%"));
           addMeasurement(m);
           m.setAttribute("type","osteoporotic_fracture_bmd");
           m.setAttribute("probability", Measurement::Value(list.at(15).toDouble(),"%"));
           addMeasurement(m);
           m.setAttribute("type","hip_fracture_bmd");
           m.setAttribute("probability", Measurement::Value(list.at(16).toDouble(),"%"));
           addMeasurement(m);

           addMetaData("type",list.at(0).toLower());
           addMetaData("country_code",list.at(1).toUInt());
           addMetaData("age",list.at(2).toDouble(),"yr");
           addMetaData("sex",list.at(3).toUInt());
           addMetaData("bmi",list.at(4).toDouble(),"kg/m2");
           addMetaData("previous_fracture",list.at(5).toUInt());
           addMetaData("parent_hip_fracture",list.at(6).toUInt());
           addMetaData("current_smoker",list.at(7).toUInt());
           addMetaData("gluccocorticoid",list.at(8).toUInt());
           addMetaData("rheumatoid_arthritis",list.at(9).toUInt());
           addMetaData("secondary_osteoporosis",list.at(10).toUInt());
           addMetaData("alcohol",list.at(11).toUInt());
           addMetaData("femoral_neck_bmd",list.at(12).toDouble());
        }
    }
}

void FraxTest::simulate()
{
    FraxMeasurement m;
    double mu = QRandomGenerator::global()->generateDouble();

    m.setAttribute("type","osteoporotic_fracture");
    double p = Utilities::interp(1.0f,30.0f,mu);
    m.setAttribute("probability", Measurement::Value(p,"%"));
    addMeasurement(m);

    m.setAttribute("type","hip_fracture");
    p = Utilities::interp(0.0f,13.0f,mu);
    m.setAttribute("probability", Measurement::Value(p,"%"));
    addMeasurement(m);

    m.setAttribute("type","osteoporotic_fracture_bmd");
    p = Utilities::interp(2.0f,22.0f,mu);
    m.setAttribute("probability", Measurement::Value(p,"%"));
    addMeasurement(m);

    m.setAttribute("type","hip_fracture_bmd");
    p = Utilities::interp(0.0f,8.0f,mu);
    m.setAttribute("probability", Measurement::Value(p,"%"));
    addMeasurement(m);
}

// String representation for debug and GUI display purposes
//
QString FraxTest::toString() const
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

bool FraxTest::isValid() const
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
    bool okTest = 4 == getNumberOfMeasurements();
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

QJsonObject FraxTest::toJsonObject() const
{
    QJsonArray jsonArr;
    foreach(auto m, m_measurementList)
    {
      jsonArr.append(m.toJsonObject());
    }
    QJsonObject json;
    if(hasMetaData())
      json.insert("test_meta_data",m_metaData.toJsonObject());
    json.insert("test_results",jsonArr);
    return json;
}
