#include "SpirometerTest.h"

#include <QDomDocument>
#include <QFile>
#include <QJsonArray>

/*

 CAVEATS/NOTES:
 - test type expected is FVC (forced vital capacity)
 - predicted and lower limit of normal values apply to the
   following variables as identified by xml tag <ResultParameter ID="X">:
   FVC, FEF2575, PEF, FEV6, FEV1, VCext, MMEF, FEV1_VCmax, FEV1_VCext,
   FEV1_FVC, FEV1_FEV6, PEF_L_Min, VCmax
 - GOLD-Hardie interpration used in pdf reporting
 - patient meta data is stored under 2 tag groups: PatientDataAtPresent and
   PatientDataAtTestTime: use the latter
 - measurements are stored by trial
 - data of the best trial values is stored as test meta data

PER TRIAL DATA:

ATI          - unitless, not required ?
AmbHumidity  - ambient_humidity, % (ambient humidity)
AmbPressure  - ambient_pressure, hPa (ambient pressure)
AmbTemp      - ambient_temperature, C (ambient temperature)
AmbTemp_Fahr - ambient_temperature_fahr, F (ambient temperature in Fahrenheit)
BEV          - bev, L (back extrapolated volume)
BTPSex       - btps_ex (BTPS [body temperature pressure saturated] factor uesd for expiration)
BTPSin       - btps_in (BTPS [body temperature pressure saturated] factor uesd for inspiration)
EOTV         - eotv, L (end of test volume)
FEF10        - fef_10, L/s (forced expiratory flow at 10% of vital capacity - synonymous with MEF90)
FEF25        - fef_25, L/s (forced expiratory flow at 25% of vital capacity - synonymous with MEF75)
FEF2575      - fef_25_75, L/s (forced expiratory flow at 25% to 75% of vital capacity - synonymous with MMEF)
FEF2575_6    - fef_25_75_6, L/s (FEF2575 based on FEV6 instead of FVC)
FEF2575_FVC  - fef_25_75_fvc, 1/s (ratio of FEF2575 to FVC)
FEF40        - fef_40, L/s (forced expiratory flow at 40% of vital capacity - synonymous with MEF60)
FEF50        - fef_50, L/s (forced expiratory flow at 50% of vital capacity - synonymous with MEF50)
FEF50_FVC    - fef_50_fvc, 1/s (ratio of FEF50 to FVC)
FEF50_VCmax  - fef_50_vcmax, 1/s (ratio of FEF50 to VCmax)
FEF60        - fef_60, L/s (forced expiratory flow at 60% of vital capacity - synoymous with MEF40)
FEF75        - fef_75, L/s (forced expiratory flow at 75% of vital capacity - synoymous with MEF25)
FEF7585      - fef_75_85, L/s (forced expiratory flow at 75% to 85% of vital capacity)
FEF80        - fef_80, L/s (forced expiratory flow at 80% of vital capacity - synoymous with MEF20)
FET          - fet, s (forced expiratory time: duration of expiratory phase)
FET2575      - fet_25_75, s (forced expiratory time between FEF25 and FEF75)
FEV1         - fev_1, L (forced expiratory volume after 1 sec)
FEV1_FEV6    - fev_1_fev_6 (ratio of FEV1 to FEV6)
FEV1_FVC     - fev_1_fvc (ratio of FEV1 to FVC)
FEV1_VCmax   - fev_1_vcmax (ratio of FEV1 to VCmax)
FEV3         - fev_3, L (forced expiratory volume after 3 sec)
FEV3_FVC     - fev_3_fvc (ratio of FEV3 to FVC)
FEV3_VCmax   - fev_3_vcmax (ratio of FEV3 to VCmax)
FEV6         - fev_6, L (forced expiratory volume after 6 sec)
FEV_25       - fev_25, L (forced expiratory volume after 0.25 sec)
FEV_5        - fev_5, L (forced expiratory volume after 0.5 sec)
FEV_5_FVC    - fev_5_fvc, (ratio of FEV_5 to FVC)
FEV_75       - fev_75, L (forced expiratory volume after 0.75 sec)
FEV_75_FEV6  - fev_75_fev_6 (ratio of FEV_75 to FEV6)
FEV_75_FVC   - fev_75_fvc (ratio of FEV_75 to FVC)
FEV_75_VCmax - fev_75_vcmax (ratio of FEV_75 to VCmax)
FVC          - fvc, L (forced expiratory vital capacity)
MEF20        - mef_20, L/s (mean expiratory flow at 80% of vital capacity - synoymous with FEF80)
MEF25        - mef_25, L/s (mean expiratory flow at 75% of vital capacity - synoymous with FEF75)
MEF40        - mef_40, L/s (mean expiratory flow at 60% of vital capacity - synoymous with FEF60)
MEF50        - mef_50, L/s (mean expiratory flow at 50% of vital capacity - synoymous with FEF50)
MEF60        - mef_60, L/s (mean expiratory flow at 40% of vital capacity - synoymous with FEF40)
MEF75        - mef_75, L/s (mean expiratory flow at 25% of vital capacity - synoymous with FEF25)
MEF90        - mef_90, L/s (mean expiratory flow at 10% of vital capacity - synoymous with FEF10)
MMEF         - mmef, L/s (mean mid-expiratory flow - synoymous with FEF2575)
MsgNo        - unitless, not required ?
MTC1         - mtc_1, 1/s ((FEF75 - FEF50) * 4 / FVC)
MTC2         - mtc_2, 1/s ((FEF50 - FEF25) * 4 / FVC)
MTC3         - mtc_3, 1/s (FEF25 * 4 / FVC)
MTCR         - mtc_r, (ratio of MTC1 to MTC3)
PEF          - pef, L/s (peak expiratory flow)
PEFT         - peft, s (time to peak flow)
PEF_L_Min    - pef_l_min, L/min (peak expiratory flow L per min)
T0           - t0, s (back-extrapolated start time of trial)
VCext        - vcext, L
VCmax        - vcmax, L (maximum vital capacity)

TRIAL META DATA:

Date                  - date
Number                - number
Rank                  - rank
RankOriginal          - rank_original
Accepted              - accepted, bool
AcceptedOriginal      - accepted_original, bool
ManualAmbientOverride - manual_ambient_override, bool

TRIAL FLOW CHANNEL DATA:

SamplingInterval      - flow_sampling_interval
TimeZeroOffset        - flow_time_zero_offset
SamplingValues        - flow_data

TRIAL VOLUME CHANNEL DATA:

SamplingInterval        - volume_sampling_interval
TimeZeroOffset          - volume_time_zero_offset
DefaultVTPlotDrawEndIdx - volume_default_vt_plot_draw_end_index
SamplingValues          - volume_data

TEST META DATA:

ID                   - patient_id
Height               - height, m
Weight               - weight, kg
Ethnicity            - ethnicity
Smoker               - smoker, bool
Asthma               - asthma, bool
Gender               - gender
DateOfBirth          - date_of_birth
COPD                 - copd, bool
LungAge              - lung_age, yr
TypeOfTest           - test_type
TestDate             - test_date
SWVersion            - software_version
QualityGradeOriginal - original_quality_grade
QualityGrade         - quality_grade
SerialNumber         - serial_number
DeviceType           - device_type
Attachment           - pdf_report_path

TEST BEST VALUES META DATA:

AmbHumidity          - ambient_humidity, %
AmbPressure          - ambient_pressure, hPa
AmbTemp              - ambient_temperature, C
AmbTemp_Fahr         - ambient_temperature_fahr, F
BEV                  - bev, L
BTPSex               - btps_ex
BTPSin               - btps_in
EOTV                 - eotv, L
FEF10                - fef_10, L/s
FEF25                - fef_25, L/s
FEF2575              - fef_25_75, fef_25_75_predicted, fef_25_75_ll_normal, L/s
FEF2575_6            - fef_25_75_6, L/s
FEF2575_FVC          - fef_25_75_fvc
FEF40                - fef_40, L/s
FEF50                - fef_50, L/s
FEF50_FVC            - fef_50_fvc
FEF50_VCmax          - fef_50_vcmax
FEF60                - fef_60, L/s
FEF75                - fef_75, L/s
FEF7585              - fef_75_85, L/s
FEF80                - fef_80, L/s
FET                  - fet, s
FET2575              - fet_25_75, s
FEV1                 - fev_1, fev_1_predicted, fev_1_ll_normal, L
FEV1_FEV6            - fev_1_fev_6, fev_1_fev_6_predicted, fev_1_fev_6_ll_normal
FEV1_FVC             - fev_1_fvc, fev_1_fvc_predicted, fev_1_fvc_ll_normal
FEV1_VCext           - *fev_1_vcext (always NaN for FVC test), fev_1_vcext_predicted, fev_1_vcext_ll_normal
FEV1_VCmax           - fev_1_vcmax, fev_1_vcmax_predicted, fev_1_vcmax_ll_normal
FEV3                 - fev_3, L
FEV3_FVC             - fev_3_fvc
FEV3_VCmax           - fev_3_vcmax
FEV6                 - fev_6, fev_6_predicted, fev_6_ll_normal, L
FEV_25               - fev_25, L
FEV_5                - fev_5, L
FEV_5_FVC            - fev_5_fvc
FEV_75               - fev_75, L
FEV_75_FEV6          - fev_75_fev_6
FEV_75_FVC           - fev_75_fvc
FEV_75_VCmax         - fev_75_vcmax
FVC                  - fvc, fvc_predicted, fvc_ll_normal, L
MEF20                - mef_20, L/s
MEF25                - mef_25, L/s
MEF40                - mef_40, L/s
MEF50                - mef_50, L/s
MEF60                - mef_60, L/s
MEF75                - mef_75, L/s
MEF90                - mef_90, L/s
MMEF                 - mmef, mmef_predicted, mmef_ll_normal, L/s
MTC1                 - mtc_1
MTC2                 - mtc_2
MTC3                 - mtc_3
MTCR                 - mtc_r
MsgNo                - not used / not required ?
PEF                  - pef, pef_predicted, pef_ll_normal, L/s
PEFT                 - peft, s
PEF_L_Min            - pef_l_min, pef_l_min_predicted, pef_l_min_ll_normal, L/min
T0                   - t0, s
VCext                - vcext, vcext_predicted, vcect_ll_normal, L  (note: value can be NaN)
VCmax                - vcmax, vcmax_predicted, vcmax_ll_normal, L

TEST PDF REPORT:

  <Command Type="TestResult">
    <Parameter Name="Attachment">C:\ProgramData\ndd\Easy on-PC\12345678.pdf</Parameter>
  </Command>

*/

const q_stringMap SpirometerTest::testMetaMap = {
    {"LungAge","lung_age"},
    {"SWVersion","device_software_version"},
    {"QualityGradeOriginal","original_quality_grade"},
    {"QualityGrade","quality_grade"},
    {"Test","test_type"},
    {"TestDate","test_date"},
    {"SerialNumber","device_serial_number"},
    {"DeviceType","device_type"}
};

const q_stringMap SpirometerTest::patientMetaMap = {
  {"Height","height"},
  {"Weight","weight"},
  {"Ethnicity","ethnicity"},
  {"Smoker","smoker"},
  {"Asthma","asthma"},
  {"Gender","gender"},
  {"DateOfBirth","date_of_birth"},
  {"COPD","copd"}
};

// the minimum output data keys required from a successful a test
//
SpirometerTest::SpirometerTest()
{
    m_outputKeyList << "patient_id";
    m_outputKeyList << "pdf_report_path";
    m_outputKeyList.append(testMetaMap.values());
    m_outputKeyList.append(patientMetaMap.values());
}

void SpirometerTest::simulate(const QVariantMap &obj)
{
  addMetaData("patient_id", obj["barcode"].toString());
  addMetaData("smoker",obj["smoker"].toBool());
  addMetaData("gender",obj["gender"].toString());
  addMetaData("height",obj["height"].toDouble(),"m");
  addMetaData("weight",obj["weight"].toDouble(),"kg");
  addMetaData("date_of_birth",obj["date_of_birth"].toDate());

  addMetaData("device_type","SPIROSON_AS");
  addMetaData("device_serial_number","200000");
  addMetaData("device_software_version","1.4.1.2");
  addMetaData("asthma",false);
  addMetaData("copd",false);
  addMetaData("ethnicity","caucasian");
  addMetaData("lung_age",54,"yr");
  addMetaData("original_quality_grade","A");
  addMetaData("pdf_report_path",QString("C:\\ProgramData\\ndd\\Easy on-PC\\%1.pdf").arg(obj["patient_id"].toString()));
  addMetaData("quality_grade","A");
  addMetaData("test_date",QDateTime::currentDateTime());
  addMetaData("test_type","FVC");

  SpirometerMeasurement best;
  best.setResultType(SpirometerMeasurement::ResultType::typeBestValues);
  best.simulate();
  if(best.isValid())
  {
    addMeasurement(best);
  }
  for(int i=0;i<3;i++)
  {
      SpirometerMeasurement trial;
      trial.setResultType(SpirometerMeasurement::ResultType::typeTrial);
      trial.simulate();
      if(trial.isValid())
      {
        addMeasurement(trial);
      }
  }
}

// parse CypressOut.xml
//
void SpirometerTest::fromFile(const QString& fileName)
{
    qDebug() << "parsing file" << fileName;
    QFile file(fileName);

    reset();
    if(!file.open(QIODevice::ReadOnly))
    {
       qDebug() << "ERROR: failed to read xml file for parsing" << fileName;
       return;
    }
    QDomDocument doc("results");
    if(!doc.setContent(&file))
    {
        file.close();
        qDebug() << "ERROR: failed to set DOM content from xml file"<< fileName;
        return;
    }
    file.close();// print out the element names of all elements that are direct children
    // of the outermost element.

    QDomElement docElem = doc.documentElement();
    QDomNode node = docElem.firstChild();
    while(!node.isNull())
    {
        QDomElement elem = node.toElement(); // try to convert the node to an element.
        if(!elem.isNull())
        {
            qDebug() << elem.tagName(); // the node really is an element.
            if("Command" == elem.tagName())
            {
              readPDFReportPath(node);
            }
            else if("Patients" == elem.tagName())
            {
              readPatients(node);

              QDomNodeList list = elem.elementsByTagName("BestValues");
              if(!list.isEmpty())
              {
                readBestValues(list.at(0));
              }
              list = elem.elementsByTagName("Trials");
              if(!list.isEmpty())
              {
                readTrials(list.at(0));
              }
            }
        }
        node = node.nextSibling();
    }
}

// TODO: generate terse output string
//
QString SpirometerTest::toString() const
{
    QString output = "";
    return output;
}

QList<QStringList> SpirometerTest::toStringListList() const
{
    QList<QStringList> data;
    if(isValid())
    {
        foreach(const auto measurement, m_measurementList)
        {
            if(SpirometerMeasurement::ResultType::typeTrial ==
               static_cast<SpirometerMeasurement>(measurement).getResultType())
            {
              if(data.isEmpty())
              {
                  qDebug() << "adding attribute keys";
                 data.append(measurement.getAttributeKeys());
              }
              qDebug() << "adding trial" << measurement.getAttribute("trial_number").toString();
              data.append(measurement.toStringList(true));
            }
        }
    }
    return data;
}

bool SpirometerTest::isValid() const
{
    bool okMeta = true;
    foreach(const auto key, m_outputKeyList)
    {
      if(!hasMetaData(key))
      {
         okMeta = false;
         qDebug() << "test invalid missing metadata" << key;
         break;
       }
    }
    bool okTest = 4 == getNumberOfMeasurements();
    qDebug() << "validating number of measurements (4)"<<getNumberOfMeasurements();
    if(okTest)
    {
      foreach(const auto m, m_measurementList)
      {
        if(!m.isValid())
        {
          qDebug() << "test invalid measurement" ;
          okTest = false;
          break;
        }
      }
    }
    return okMeta && okTest;
}

// String keys are converted to snake_case
//
QJsonObject SpirometerTest::toJsonObject() const
{
    QJsonArray trialJson;
    QJsonObject bestJson;
    foreach(const auto measurement, m_measurementList)
    {
      if(SpirometerMeasurement::ResultType::typeBestValues ==
         static_cast<SpirometerMeasurement>(measurement).getResultType())
        bestJson = measurement.toJsonObject();
      else
        trialJson.append(measurement.toJsonObject());
    }
    QJsonObject json;
    if(hasMetaData())
    {
      QJsonObject deviceJson;
      QJsonObject metaJson = m_metaData.toJsonObject();
      deviceJson.insert("device_type",metaJson.take("device_type"));
      deviceJson.insert("device_serial_number",metaJson.take("device_serial_number"));
      deviceJson.insert("device_software_version",metaJson.take("device_software_version"));
      json.insert("device_data", deviceJson);
      json.insert("test_meta_data", metaJson);
    }
    json.insert("test_trials", trialJson);
    json.insert("test_best_values",bestJson);
    return json;
}

void SpirometerTest::readPDFReportPath(const QDomNode& node)
{
    QDomElement child = node.firstChildElement("Parameter");
    if(child.isNull())
      return;

    qDebug() << child.hasAttribute("Name") << child.tagName() << child.text() << child.attribute("Name");
    if(child.hasAttribute("Name") && "Attachment" == child.attribute("Name"))
      addMetaData("pdf_report_path",child.text());
}

void SpirometerTest::readPatients(const QDomNode& node)
{
    QDomElement child = node.firstChildElement("Patient");
    if(child.isNull())
      return;

    if(child.hasAttribute("ID" ))
      addMetaData("patient_id",child.attribute("ID"));

    foreach(const auto tag, testMetaMap.toStdMap())
    {
      QDomNodeList list = child.elementsByTagName(tag.first);
      if(!list.isEmpty())
      {
        QDomElement elem = list.item(0).toElement();
        if("Test" == tag.first && elem.hasAttribute("TypeOfTest"))
           addMetaData(tag.second,elem.attribute("TypeOfTest"));
        else if("LungAge" == tag.first)
        {
          addMetaData(tag.second,elem.text().toInt(),"yr");
        }
        else
          addMetaData(tag.second,elem.text());
      }
    }

    QDomNodeList list = child.elementsByTagName("PatientDataAtTestTime");
    list = list.item(0).childNodes();
    if(!list.isEmpty())
    {
      for(int i=0; i<list.count(); i++)
      {
        QDomElement elem = list.item(i).toElement();
        QString tag = elem.tagName();
        if(patientMetaMap.contains(tag))
        {
          QString key = patientMetaMap[tag];
          QString s = elem.text().toLower();

          if(!s.isEmpty())
          {
            if("no" == s || "yes" == s)
              addMetaData(key,"no" ==s ? false : true);
            else
            {
              if("height" == key)
              {
                addMetaData(key,s.toDouble(),"m");
              }
              else if("weight" == key)
              {
                addMetaData(key,s.toDouble(),"kg");
              }
              else
                addMetaData(key,s);
            }
          }
          else
          {
            addMetaData(key,QVariant(false));
          }
       }
      }
    }
}

void SpirometerTest::readTrials(const QDomNode& node)
{
    QDomNodeList trialNodeList = node.childNodes();
    if(trialNodeList.isEmpty())
    {
        qDebug() << "no trial child nodes";
        return;
    }

    qDebug() << "n trials" << trialNodeList.size();
    for(int i = 0; i < trialNodeList.size(); i++)
    {
        SpirometerMeasurement trial;
        trial.setResultType(SpirometerMeasurement::ResultType::typeTrial);

        QDomNode trialNode = trialNodeList.at(i);
        if("Trial" != trialNode.toElement().tagName()) continue;
        QDomNodeList nodeList = trialNode.childNodes();
        for(int j=0; j<nodeList.size(); j++)
        {
          QDomNode node = nodeList.at(j);
          QDomElement elem = node.toElement();
          QString tag = elem.tagName();
          if("ResultParameters" == tag)
          {
            readParameters(node, &trial);
          }
          else if(tag.startsWith("Channel"))
          {
            readChannel(node, &trial);
          }
          else if(SpirometerMeasurement::trialMap.contains(tag))
          {
            QString s = elem.text();
            if(s.isEmpty())
            {
              trial.setAttribute(SpirometerMeasurement::trialMap[tag],QVariant());
            }
            else
            {
              if("true" == s || "false" == s)
              {
                trial.setAttribute(SpirometerMeasurement::trialMap[tag],"true" == s ? true : false);
              }
              else
              {
                if("Date" == tag)
                  trial.setAttribute(SpirometerMeasurement::trialMap[tag],
                    QDateTime::fromString(s,"yyyy-MM-dd'T'hh:mm:ss.z"));
                else
                  trial.setAttribute(SpirometerMeasurement::trialMap[tag],s.toInt());
              }
            }
          }
        }
        if(trial.isValid())
        {
          addMeasurement(trial);
        }
        else
          qDebug() << "ERROR: failed to add trial";
    }
}

void SpirometerTest::readBestValues(const QDomNode& node)
{
  SpirometerMeasurement best;
  best.setResultType(SpirometerMeasurement::ResultType::typeBestValues);
  readParameters(node, &best);
  qDebug() << "read best values";
  if(best.isValid())
  {
    qDebug() << "OK best values";
    addMeasurement(best);
  }
  else
      qDebug() << "ERROR: failed to add best values";
}

void SpirometerTest::readParameters(const QDomNode& node, SpirometerMeasurement* measure)
{
    // every element has the following pattern
    /**
    <ResultParameter ID="SomeParameter">
      <DataValue>X</DataValue>     <- all values are numeric and can be doubles
      <Unit>Y</Unit>               <- or <Unit />
      <PredictedValue>NaN</PredictedValue> <- or a value instead of NaN
      <LLNormalValue>NaN</LLNormalValue>   <- or a value instead of NaN
    </ResultParameter>
     */

    QDomNodeList list = node.childNodes();
    if(list.isEmpty())
    {
        qDebug() << "no parameter child nodes";
        return;
    }
    for(int i=0; i<list.count(); i++)
    {
       QDomNode child = list.item(i);
       QDomElement elem = child.toElement();
       if(elem.hasAttribute("ID"))
       {
           QString tag = elem.attribute("ID");
           if(SpirometerMeasurement::resultMap.contains(tag))
           {
               QString key = SpirometerMeasurement::resultMap[tag];
               QDomNodeList clist = child.childNodes();
               QJsonObject arr;
               QJsonObject pred;
               QJsonObject norm;
               for(int j=0; j<clist.size(); j++)
               {
                   QDomElement celem = clist.item(j).toElement();
                   QString ctag = celem.tagName();
                   QString s = celem.text();
                   if(SpirometerMeasurement::parameterList.contains(ctag) && !s.isEmpty() && "NaN" != s)
                   {
                       if("DataValue"==ctag)
                       {
                           arr.insert("value",QJsonValue::fromVariant(s.toDouble()));
                       }
                       else if("Unit"==ctag)
                       {
                           arr.insert("units",s);
                           pred.insert("units",s);
                           norm.insert("units",s);
                       }
                       else if("PredictedValue"==ctag)
                       {
                           pred.insert("value",QJsonValue::fromVariant(s.toDouble()));
                       }
                       else if("LLNormalValue"==ctag)
                       {
                           norm.insert("value",QJsonValue::fromVariant(s.toDouble()));
                       }
                   }
               }
               if(arr.contains("value"))
               {
                   if(arr.contains("units"))
                     measure->setAttribute(key,arr["value"].toDouble(),arr["units"].toString());
                   else
                     measure->setAttribute(key,arr["value"].toDouble());
               }
               if(pred.contains("value"))
               {
                   QString predName = QString("%1_predicted").arg(key);
                   if(pred.contains("units"))
                     measure->setAttribute(predName,pred["value"].toDouble(),pred["units"].toString());
                   else
                     measure->setAttribute(predName,pred["value"].toDouble());
               }
               if(norm.contains("value"))
               {
                   QString normName = QString("%1_ll_normal").arg(key);
                   if(norm.contains("units"))
                     measure->setAttribute(normName,norm["value"].toDouble(),norm["units"].toString());
                   else
                     measure->setAttribute(normName,norm["value"].toDouble());
               }
           }
       }
    }
}

// channel data is not meta data and is thereore applicable to an actual measurement (ie., trial)
//
void SpirometerTest::readChannel(const QDomNode& node, SpirometerMeasurement* measure)
{
    QDomElement elem = node.toElement();
    QString prefix = elem.tagName().mid(7).toLower();
    qDebug() << "reading" << prefix << "channel";

    foreach(const auto tag, SpirometerMeasurement::channelMap.toStdMap())
    {
      QDomNodeList list = elem.elementsByTagName(tag.first);
      if(!list.isEmpty())
      {
        QDomElement elem = list.item(0).toElement();
        QString key = QString("%1_%2").arg(prefix,tag.second);

        // only insert the sampling values if the xml element has the correct attribute
        if("SamplingValues" == tag.first)
        {
          if(elem.hasAttribute("TypeOfData"))
          {
            QStringList values = elem.text().split(" ");
            measure->setAttribute(key,values.join(","));
            QString numKey = QString("%1_value_count").arg(prefix);
            measure->setAttribute(numKey,values.count());
          }
        }
        else if("SamplingInterval" == tag.first || "TimeZeroOffset" == tag.first)
        {
          measure->setAttribute(key,elem.text().toDouble());
        }
        else
        {
          measure->setAttribute(key,elem.text().toInt());
        }
      }
    }
}
