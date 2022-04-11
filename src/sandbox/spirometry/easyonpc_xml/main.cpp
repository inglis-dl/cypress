#include <QCoreApplication>
#include <QDebug>
#include <QDomDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>

typedef QMap<QString,QString> q_stringMap;

const  q_stringMap testMetaMap = {
    {"LungAge","lung_age"},
    {"SWVersion","software_version"},
    {"QualityGradeOriginal","original_quality_grade"},
    {"QualityGrade","quality_grade"},
    {"Test","test_type"},
    {"TestDate","test_date"},
    {"SerialNumber","serial_number"},
    {"DeviceType","device_type"}
};

const q_stringMap patientMetaMap = {
  {"Height","height"},
  {"Weight","weight"},
  {"Ethnicity","ethnicity"},
  {"Smoker","smoker"},
  {"Asthma","asthma"},
  {"Gender","gender"},
  {"DateOfBirth","date_of_birth"},
  {"COPD","copd"}
};

const QStringList paramList = {"DataValue","Unit","PredictedValue","LLNormalValue"};

const q_stringMap channelMap = {
    {"SamplingInterval","sampling_interval"},
    {"SamplingValues","values"},
    {"TimeZeroOffset","time_zero_offset"},
    {"DefaultVTPlotDrawEndIdx","default_vt_plot_draw_end_index"}
};

const q_stringMap resultMap = {
    {"AmbHumidity","ambient_humidity"},
    {"AmbPressure","ambient_pressure"},
    {"AmbTemp","ambient_temperature"},
    {"AmbTemp_Fahr","ambient_temperature_fahr"},
    {"BEV","bev"},
    {"BTPSex","btps_ex"},
    {"BTPSin","btps_in"},
    {"EOTV","eotv"},
    {"FEF10","fef_10"},
    {"FEF25","fef_25"},
    {"FEF2575","fef_25_75"},
    {"FEF2575_6","fef_25_75_6"},
    {"FEF2575_FVC","fef_25_75_fvc"},
    {"FEF40","fef_40"},
    {"FEF50","fef_50"},
    {"FEF50_FVC","fef_50_fvc"},
    {"FEF50_VCmax","fef_50_vcmax"},
    {"FEF60","fef_60"},
    {"FEF75","fef_75"},
    {"FEF7585","fef_75_85"},
    {"FEF80","fef_80"},
    {"FET","fet"},
    {"FET2575","fet_25_75"},
    {"FEV1","fev_1"},
    {"FEV1_FEV6","fev_1_fev_6"},
    {"FEV1_FVC","fev_1_fvc"},
    {"FEV1_VCext","fev_1_vcext"},
    {"FEV1_VCmax","fev_1_vcmax"},
    {"FEV3","fev_3"},
    {"FEV3_FVC","fev_3_fvc"},
    {"FEV3_VCmax","fev_3_vcmax"},
    {"FEV6","fev_6"},
    {"FEV_25","fev_25"},
    {"FEV_5","fev_5"},
    {"FEV_5_FVC","fev_5_fvc"},
    {"FEV_75","fev_75"},
    {"FEV_75_FEV6","fev_75_fev_6"},
    {"FEV_75_FVC","fev_75_fvc"},
    {"FEV_75_VCmax","fev_75_vcmax"},
    {"FVC","fvc"},
    {"MEF20","mef_20"},
    {"MEF25","mef_25"},
    {"MEF40","mef_40"},
    {"MEF50","mef_50"},
    {"MEF60","mef_60"},
    {"MEF75","mef_75"},
    {"MEF90","mef_90"},
    {"MMEF","mmef"},
    {"MTC1","mtc_1"},
    {"MTC2","mtc_2"},
    {"MTC3","mtc_3"},
    {"MTCR","mtc_r"},
    {"MsgNo","msg_no"},
    {"PEF","pef"},
    {"PEFT","peft"},
    {"PEF_L_Min","pef_l_min"},
    {"T0","t0"},
    {"VCext","vcext"},
    {"VCmax","vcmax"}
};

const q_stringMap trialMap =
{
  {"Date","trial_date"},
  {"Number","trial_number"},
  {"Rank","trial_rank"},
  {"RankOriginal","trial_rank_original"},
  {"Accepted","trial_accpeted"},
  {"AcceptedOriginal","trial_accepted_original"},
  {"ManualAmbientOverride","trial_manual_ambient_override"}
};

// path to pdf output file
QJsonObject readTestResult(const QDomNode& node)
{
    QJsonObject obj;
    QDomElement child = node.firstChildElement("Parameter");
    if(child.isNull())
    {
        return obj;
    }
    qDebug() << child.hasAttribute("Name") << child.tagName() << child.text() << child.attribute("Name");
    if(child.hasAttribute("Name") && "Attachment" == child.attribute("Name"))
        obj.insert("pdf_report_path",child.text());
    return obj;
}

QJsonObject readPatients(const QDomNode& node)
{
    QJsonObject obj;
    qDebug() << "nchildren" << node.childNodes().size();
    QDomElement child = node.firstChildElement("Patient");
    if(child.isNull())
    {
        return obj;
    }
    qDebug() << child.hasAttribute("ID") << child.tagName() << child.attribute("ID");
    if(child.hasAttribute("ID" ))
      obj.insert("patient_id",child.attribute("ID"));

    foreach(const auto tag, testMetaMap.toStdMap())
    {
      QDomNodeList list = child.elementsByTagName(tag.first);
      if(!list.isEmpty())
      {
        QDomElement elem = list.item(0).toElement();
        if("Test" == tag.first && elem.hasAttribute("TypeOfTest"))
           obj.insert(tag.second,elem.attribute("TypeOfTest"));
        else if("LungAge" == tag.first)
        {
          QJsonObject arr;
          arr.insert("value",QJsonValue::fromVariant(elem.text().toInt()));
          arr.insert("units","yr");
          obj.insert(tag.second,arr);
        }
        else
          obj.insert(tag.second,elem.text());
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
              obj.insert(key,"no" ==s ? false : true);
            else
            {
              if("height" == key)
              {
                QJsonObject arr;
                arr.insert("value",QJsonValue::fromVariant(s.toDouble()));
                arr.insert("units","m");
                obj.insert(key,arr);
              }
              else if("weight" == key)
              {
                QJsonObject arr;
                arr.insert("value",QJsonValue::fromVariant(s.toInt()));
                arr.insert("units","kg");
                obj.insert(key,arr);
              }
              else
                obj.insert(key,s);
            }
          }
          else
            obj.insert(key,QJsonValue());
       }
     }
    }
    return obj;
}

QJsonObject readResultParameters(const QDomNode& node)
{
  QJsonObject obj;
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
      qDebug() << "no best values child nodes";
      return obj;
  }
  qDebug() << "reading list of best values"<<list.size();
  for(int i=0; i<list.count(); i++)
  {
     QDomNode child = list.item(i);
     QDomElement elem = child.toElement();
     if(elem.hasAttribute("ID"))
     {
         QString tag = elem.attribute("ID");
         qDebug() << tag;
         if(resultMap.contains(tag))
         {
             QString key = resultMap[tag];
             QDomNodeList clist = child.childNodes();
             QJsonObject arr;
             QJsonObject pred;
             QJsonObject norm;
             for(int j=0; j<clist.size(); j++)
             {
                 QDomElement celem = clist.item(j).toElement();
                 QString ctag = celem.tagName();
                 QString s = celem.text();
                 if(paramList.contains(ctag) && !s.isEmpty() && "NaN" != s)
                 {
                     if("DataValue"==ctag)
                     {
                         qDebug() << "value"<<s;
                         arr.insert("value",QJsonValue::fromVariant(s.toDouble()));
                     }
                     else if("Unit"==ctag)
                     {
                         qDebug() <<"units"<<s;
                         arr.insert("units",s);
                         pred.insert("units",s);
                         norm.insert("units",s);
                     }
                     else if("PredictedValue"==ctag)
                     {
                         qDebug()<<"predicted"<<s;
                         pred.insert("value",QJsonValue::fromVariant(s.toDouble()));
                     }
                     else if("LLNormalValue"==ctag)
                     {
                         qDebug()<<"llnormal"<<s;
                         norm.insert("value",QJsonValue::fromVariant(s.toDouble()));
                     }
                 }
             }
             if(arr.contains("value"))
             {
                 qDebug() << "insert"<< key;
                 if(arr.contains("units"))
                   obj.insert(key,arr);
                 else
                   obj.insert(key,arr["value"]);
             }
             if(pred.contains("value"))
             {
                 QString predName = QString("%1_predicted").arg(key);
                 qDebug() << "insert"<<predName;
                 if(pred.contains("units"))
                   obj.insert(predName,pred);
                 else
                   obj.insert(predName,pred["value"]);
             }
             if(norm.contains("value"))
             {
                 QString normName = QString("%1_ll_normal").arg(key);
                 qDebug() << "insert"<<normName;
                 if(norm.contains("units"))
                   obj.insert(normName,norm);
                 else
                   obj.insert(normName,norm["value"]);
             }
         }
     }

  }
  return obj;
}

QJsonObject readChannel(const QDomNode& node)
{
    QJsonObject obj;
    QDomElement elem = node.toElement();
    QString prefix = elem.tagName().mid(7).toLower();
    qDebug() << "channel" << prefix;

    foreach(const auto tag, channelMap.toStdMap())
    {
      QDomNodeList list = elem.elementsByTagName(tag.first);
      if(!list.isEmpty())
      {
        QDomElement e = list.item(0).toElement();
        QString key = QString("%1_%2").arg(prefix,tag.second);
        if("SamplingValues" == tag.first && e.hasAttribute("TypeOfData"))
          obj.insert(key,e.text());
        else
          obj.insert(key,e.text());
      }
    }
    return obj;
}

QJsonObject readTrials(const QDomNode& node)
{
  QJsonObject obj;
  QDomNodeList trialNodeList = node.childNodes();
  if(trialNodeList.isEmpty())
  {
      qDebug() << "no trial child nodes";
      return obj;
  }

  qDebug() << "n trials" << trialNodeList.size();
  QJsonArray arr;
  for(int i = 0; i < trialNodeList.size(); i++)
  {
      QDomNode trialNode = trialNodeList.at(i);
      if("Trial" != trialNode.toElement().tagName()) continue;
      QDomNodeList nodeList = trialNode.childNodes();
      qDebug() << "trial" << (i+1) << ", n nodes"<<nodeList.size();
      QJsonObject trialJson;
      for(int j=0; j<nodeList.size(); j++)
      {
        QDomNode node = nodeList.at(j);
        QDomElement elem = node.toElement();
        QString tag = elem.tagName();
        qDebug() << "trial"<<(i+1)<<"elem"<<j<<tag;
        if("ResultParameters" == tag)
        {
           QJsonObject params = readResultParameters(node);
           QStringList keys = params.keys();
           foreach(const auto key, keys)
               trialJson.insert(key,params[key]);
        }
        else if(tag.startsWith("Channel"))
        {
            QJsonObject channel = readChannel(node);
            QStringList keys = channel.keys();
            foreach(const auto key, keys)
                trialJson.insert(key,channel[key]);
        }
        else if(trialMap.contains(tag))
        {
          QString s = elem.text();
          if(s.isEmpty())
          {
            trialJson.insert(trialMap[tag],QJsonValue());
          }
          else
          {
            if("true" == s || "false" == s)
            {
              trialJson.insert(trialMap[tag],QJsonValue(("true" == s ? true : false)));
            }
            else
            {
              if("Date" == tag)
                trialJson.insert(trialMap[tag],QJsonValue::fromVariant(
                  QDateTime::fromString(s,"yyyy-MM-dd'T'hh:mm:ss.z")));
              else
                trialJson.insert(trialMap[tag],QJsonValue::fromVariant(s.toInt()));
            }
          }
        }
      }
      arr.append(trialJson);
  }
  obj.insert("test_trials",arr);

  return obj;
}

int main(int argc, char *argv[])
{
    if(2 == argc)
    {
        QString fileName = argv[1];
        qDebug() << "parsing file" << fileName;
        QFile ifile(fileName);    QStringList keys = {
            "Hour", "Minute"
        };

        if(!ifile.open(QIODevice::ReadOnly))
        {
           qDebug() << "ERROR: failed to read xml file for parsing" << fileName;
           return EXIT_FAILURE;
        }
        QDomDocument doc("spirometryDocument");
        if(!doc.setContent(&ifile))
        {
            ifile.close();
            qDebug() << "ERROR: failed to set DOM content from xml file"<< fileName;
            return EXIT_FAILURE;
        }
        ifile.close();// print out the element names of all elements that are direct children
        // of the outermost element.
        QDomElement docElem = doc.documentElement();
        QDomNode n = docElem.firstChild();

        QJsonObject metaJson;
        QJsonObject testJson;
        while(!n.isNull())
        {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull())
            {
                qDebug() << e.tagName(); // the node really is an element.
                if("Command" == e.tagName())
                {
                    QJsonObject obj = readTestResult(n);
                    if(!obj.isEmpty())
                    {
                        QStringList keys = obj.keys();
                        foreach(const auto key, keys)
                          metaJson.insert(key, obj[key]);
                    }
                }
                else if("Patients" == e.tagName())
                {
                    QJsonObject obj = readPatients(n);
                    if(!obj.isEmpty())
                    {
                        QStringList keys = obj.keys();
                        foreach(const auto key, keys)
                          metaJson.insert(key, obj[key]);
                    }

                    QDomNodeList list = e.elementsByTagName("BestValues");
                    if(!list.isEmpty())
                    {
                        obj = readResultParameters(list.at(0));
                        if(!obj.isEmpty())
                        {
                            metaJson.insert("best_values",obj);
                        }
                    }
                    list = e.elementsByTagName("Trials");
                    if(!list.isEmpty())
                    {
                        obj = readTrials(list.at(0));
                        if(!obj.isEmpty())
                        {
                            testJson = obj;
                        }
                     }
                }
            }
            n = n.nextSibling();
        }
        if(!metaJson.isEmpty() && !testJson.isEmpty())
        {
            QJsonObject deviceJson;
            if(metaJson.contains("serial_number"))
            {
                deviceJson.insert("serial_number",metaJson.take("serial_number"));
            }
            if(metaJson.contains("software_version"))
            {
                deviceJson.insert("software_version",metaJson.take("software_version"));
            }
            if(metaJson.contains("device_type"))
            {
                deviceJson.insert("device_type",metaJson.take("device_type"));
            }
            if(metaJson.contains("best_values"))
            {
                testJson.insert("best_values",metaJson.take("best_values"));
            }

            QJsonObject outputJson;
            if(!deviceJson.isEmpty())
                outputJson.insert("device",deviceJson);
            if(!metaJson.isEmpty())
                outputJson.insert("test_meta_data",metaJson);
            if(!testJson.isEmpty())
                outputJson.insert("test_results",testJson);

            QFile saveFile("test.json");
            saveFile.open(QIODevice::WriteOnly);
            saveFile.write(QJsonDocument(outputJson).toJson());
            saveFile.close();
        }
        else
            qDebug() <<"ERROR: failed to parse"<<fileName;
    }

    return EXIT_SUCCESS;
}
