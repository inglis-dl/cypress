#include <QCoreApplication>
#include <QDebug>
#include <QDomDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>

QMap<QString,QJsonObject> data;
QList<QString> xmlKeys = {
    "ObservationType",    // ignore
    "ObservationDateTime",
       // test_datetime <
       //
    "UID",                // ignore
    "ClinicalInfo",
      // device_name, software_version
    "PatientVisit",       // ignore
    "PatientInfo",
      // pacemaker bool
    "FilterSetting",
       // filter_settings
       //    cubic_spline bool
       //
    "Device-Type",  // ignore
    "Interpretation",
    // Diagnosis
    //  DiagnosisText
    "RestingECGMeasurements",
    "VectorLoops",  // ignore
    "StripData", // ignore
    "Export",    // ignore
    "CSWeb"      // ignore
};

QJsonObject readObservationDatetime(const QDomNode&);
QJsonObject readClinicalInfo(const QDomNode&);
QJsonObject readPatientInfo(const QDomNode&);
QJsonObject readFilterSetting(const QDomNode&);
QJsonObject readInterpretation(const QDomNode&);
QJsonObject readRestingECGMeasurements(const QDomNode&);

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
        QDomDocument doc("ecgDocument");
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

        QJsonArray arr;
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QJsonObject obj;
                qDebug() << e.tagName(); // the node really is an element.
                if("Interpretation" == e.tagName())
                    obj = readInterpretation(n);
                else if("ClinicalInfo" == e.tagName())
                    obj = readClinicalInfo(n);
                else if("FilterSetting" == e.tagName())
                    obj = readFilterSetting(n);
                else if("ObservationDateTime" == e.tagName())
                    obj = readObservationDatetime(n);
                else if("PatientInfo" == e.tagName())
                    obj = readPatientInfo(n);
                else if("Interpretation" == e.tagName())
                    obj = readInterpretation(n);
                else if("RestingECGMeasurements" == e.tagName())
                    obj = readRestingECGMeasurements(n);

                if(!obj.isEmpty())
                    arr.append(obj);
            }
            n = n.nextSibling();
        }
        if(!arr.isEmpty())
        {
            QFile saveFile("test.json");
            saveFile.open(QIODevice::WriteOnly);
            saveFile.write(QJsonDocument(arr).toJson());
            saveFile.close();
        }
        else
            qDebug() <<"ERROR: failed to parse"<<fileName;
    }

    return EXIT_SUCCESS;
}

QJsonObject readRestingECGMeasurements(const QDomNode& node)
{
    QJsonObject obj;
    QDomNodeList list = node.childNodes();
    if(list.isEmpty())
    {
        return obj;
    }
    QMap<QString,QString> map = {
        {"VentricularRate","ventricular_rate"},
        {"PQInterval","PQ_interval"},
        {"PDuration","P_duration"},
        {"QRSDuration","QRS_duration"},
        {"QTInterval","QT_interval"},
        {"QTCInterval","QTC_interval"},
        {"RRInterval","RR_interval"},
        {"PPInterval","PP_interval"},
        {"SokolovLVHIndex","Sokolov_LVH_index"},
        {"PAxis","P_axis"},
        {"RAxis","R_axis"},
        {"TAxis","T_axis"},
        {"QTDispersion","QT_dispersion"},
        {"QTDispersionBazett","QT_dispersion_Bazett"},
        {"QRSNum","QRS_number"},
        {"POnset","P_onset"},
        {"POffset","P_offset"},
        {"QOnset","Q_onset"},
        {"QOffset","Q_offset"},
        {"TOffset","T_offset"}
    };
    for(int i=0; i<list.count(); i++)
    {
       QDomElement elem = list.item(i).toElement();
       QString tag = elem.tagName();
       if(map.contains(tag))
       {
          QString s = elem.text().toLower();
          if(!s.isEmpty())
          {
            if("no" == s || "yes" == s)
              obj.insert(map[tag],"no" ==s ? false : true);
            else
            {
                if(elem.hasAttribute("units"))
                {
                   QJsonObject arr;
                   arr.insert("value",QJsonValue::fromVariant(s.toInt()));
                   arr.insert("units",elem.attribute("units"));
                   obj.insert(map[tag],arr);
                }
                else
                   obj.insert(map[tag],s);
            }
          }
       }
    }

    QJsonObject json;
    json.insert("resting_ecg_measurements", obj);
    return json;
}

QJsonObject readObservationDatetime(const QDomNode& node)
{
    QJsonObject obj;
    QDomNodeList list = node.childNodes();
    if(list.isEmpty())
    {
        return obj;
    }
    QMap<QString,QString> map = {
        {"Hour",""},
        {"Minute",""},
        {"Second",""},
        {"Day",""},
        {"Month",""},
        {"Year",""}};
    for(int i=0; i<list.count(); i++)
    {
       QDomElement elem = list.item(i).toElement();
       QString tag = elem.tagName();
       if(map.contains(tag))
       {
          QString s = elem.text();
          if(!s.isEmpty())
          {
              map[tag] = s.simplified();
          }
       }
    }

    QString s = QString("%1-%2-%3 %4:%5:%6").arg(
                    map["Year"],
                    map["Month"],
                    map["Day"],
                    map["Hour"],
                    map["Minute"],
                    map["Second"]
                    );
    QDateTime d = QDateTime::fromString(s, "yyyy-M-d hh:mm:ss");
    obj.insert("observation_datetime",QJsonValue::fromVariant(d));
    qDebug() << "ok, set observation datetime"<<d.toString("yyyy-MM-dd hh:mm:ss");
    return obj;
}

QJsonObject readClinicalInfo(const QDomNode& node)
{
    QJsonObject obj;
    QDomElement child = node.firstChildElement("DeviceInfo");
    if(child.isNull())
    {
        return obj;
    }
    QDomNodeList list = child.childNodes();
    if(list.isEmpty())
    {
        return obj;
    }
    QMap<QString,QString> map = {
        {"Desc","device_description"},
        {"SoftwareVer","software_version"},
        {"AnalysisVer","analysis_version"}};
    for(int i=0; i<list.count(); i++)
    {
       QDomElement elem = list.item(i).toElement();
       QString tag = elem.tagName();
       if(map.contains(tag))
       {
          QString s = elem.text().toLower();
          if(!s.isEmpty())
          {
            if("no" == s || "yes" == s)
              obj.insert(map[tag],"no" ==s ? false : true);
            else
            {
                if(elem.hasAttribute("units"))
                {
                   QJsonObject arr;
                   arr.insert("value",QJsonValue::fromVariant(s.toInt()));
                   arr.insert("units",elem.attribute("units"));
                   obj.insert(map[tag],arr);
                }
                else
                   obj.insert(map[tag],s);
            }
          }
       }
    }

    QJsonObject json;
    json.insert("device_info", obj);
    return obj;
}

QJsonObject readPatientInfo(const QDomNode& node)
{
    QJsonObject obj;
    QDomNodeList list = node.childNodes();
    if(list.isEmpty())
    {
        return obj;
    }
    QStringList keys = {
      "Gender","Race","PaceMaker" };
    for(int i=0; i<list.count(); i++)
    {
       QDomElement elem = list.item(i).toElement();
       QString tag = elem.tagName();
       if(keys.contains(tag))
       {
          QString s = elem.text().toLower();
          if(!s.isEmpty())
          {
            if("no" == s || "yes" == s)
              obj.insert(tag.toLower(),"no" ==s ? false : true);
            else
              obj.insert(tag.toLower(),s);
          }
       }
    }

    QJsonObject json;
    json.insert("patient_info", obj);
    return obj;
}

QJsonObject readFilterSetting(const QDomNode& node)
{
    QJsonObject obj;
    QDomNodeList list = node.childNodes();
    if(list.isEmpty())
    {
        return obj;
    }
    QMap<QString,QString> map = {
        {"CubicSpline","cubic_spline"},
        {"Filter50Hz","filter_50_Hz"},
        {"Filter60Hz","filter_60_Hz"},
        {"LowPass","low_pass"}};
    for(int i=0; i<list.count(); i++)
    {
       QDomElement elem = list.item(i).toElement();
       QString tag = elem.tagName();
       if(map.contains(tag))
       {
          QString s = elem.text().toLower();
          if(!s.isEmpty())
          {
            if("no" == s || "yes" == s)
              obj.insert(map[tag],"no" ==s ? false : true);
            else
            {
                if(elem.hasAttribute("units"))
                {
                   QJsonObject arr;
                   arr.insert("value",QJsonValue::fromVariant(s.toInt()));
                   arr.insert("units",elem.attribute("units"));
                   obj.insert(map[tag],arr);
                }
                else
                   obj.insert(map[tag],s);
            }
          }
       }
    }

    QJsonObject json;
    json.insert("filter_settings", obj);
    return json;
}

QJsonObject readInterpretation(const QDomNode& node)
{
    QJsonObject obj;
    QDomElement child = node.firstChildElement("Diagnosis");
    if(child.isNull())
    {
        return obj;
    }
    QDomNodeList list = child.elementsByTagName("DiagnosisText");
    if(list.isEmpty())
    {
        return obj;
    }
    QJsonArray arr;
    for(int i=0; i<list.size(); i++)
    {
      QDomElement elem = list.item(i).toElement();
      if(!elem.text().isEmpty())
      {
        arr.append(elem.text());
      }
    }
    if(arr.isEmpty())
    {
        return obj;
    }
    if(1 < arr.size())
    {
        obj.insert("diagnosis",arr);
    }
    else
    {
        obj.insert("diagnosis",arr.first());
    }
    return obj;
}
