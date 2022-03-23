#include "ECGTest.h"

#include <QDateTime>
#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QDomDocument>
#include "../auxiliary/Utilities.h"

ECGTest::ECGTest()
{
    m_outputKeyList << "observation_datetime";
    m_outputKeyList << "device_description";
    m_outputKeyList << "software_version";
    m_outputKeyList << "gender";
    m_outputKeyList << "race";
    m_outputKeyList << "pacemaker";
    m_outputKeyList << "cubic_spline";
    m_outputKeyList << "filter_50_Hz";
    m_outputKeyList << "filter_60_Hz";
    m_outputKeyList << "low_pass";
}

void ECGTest::fromFile(const QString &fileName)
{
    QFile ifile(fileName);
    if(!ifile.open(QIODevice::ReadOnly))
    {
       qDebug() << "ERROR: failed to read xml file for parsing" << fileName;
       return;
    }
    QDomDocument doc("ecgDocument");
    if(!doc.setContent(&ifile))
    {
        ifile.close();
        qDebug() << "ERROR: failed to set DOM content from xml file"<< fileName;
        return;
    }
    ifile.close();

    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();
    reset();
    ECGMeasurement m;
    while(!n.isNull())
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull())
        {
            qDebug() << e.tagName(); // the node really is an element.

            if("ClinicalInfo" == e.tagName())
                readClinicalInfo(n);
            else if("FilterSetting" == e.tagName())
                readFilterSetting(n);
            else if("ObservationDateTime" == e.tagName())
                readObservationDatetime(n);
            else if("PatientInfo" == e.tagName())
                readPatientInfo(n);
            else if("Interpretation" == e.tagName())
                m.fromDomNode(n);
            else if("RestingECGMeasurements" == e.tagName())
                m.fromDomNode(n);
        }
        n = n.nextSibling();
    }
    if(m.isValid())
        addMeasurement(m);
}

void ECGTest::readObservationDatetime(const QDomNode& node)
{
    QDomNodeList list = node.childNodes();
    if(list.isEmpty())
    {
        return;
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
    addMetaData("observation_datetime",d);
}

void ECGTest::readClinicalInfo(const QDomNode& node)
{
    QDomElement child = node.firstChildElement("DeviceInfo");
    if(child.isNull())
    {
        return;
    }
    QDomNodeList list = child.childNodes();
    if(list.isEmpty())
    {
        return;
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
              addMetaData(map[tag],"no" ==s ? false : true);
            else
            {
                if(elem.hasAttribute("units"))
                {
                   addMetaData(map[tag],s.toInt(),elem.attribute("units"));
                }
                else
                   addMetaData(map[tag],s);
            }
          }
       }
    }

}

void ECGTest::readPatientInfo(const QDomNode& node)
{
    QDomNodeList list = node.childNodes();
    if(list.isEmpty())
    {
        return;
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
              addMetaData(tag.toLower(),"no" ==s ? false : true);
            else
              addMetaData(tag.toLower(),s);
          }
       }
    }
}

void ECGTest::readFilterSetting(const QDomNode& node)
{
    QDomNodeList list = node.childNodes();
    if(list.isEmpty())
    {
        return;
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
              addMetaData(map[tag],"no" ==s ? false : true);
            else
            {
                if(elem.hasAttribute("units"))
                {
                   addMetaData(map[tag],s.toInt(),elem.attribute("units"));
                }
                else
                   addMetaData(map[tag],s);
            }
          }
       }
    }
}

void ECGTest::simulate(const QJsonObject& input)
{
    Q_UNUSED(input)
}

// String representation for debug and GUI display purposes
//
QString ECGTest::toString() const
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

bool ECGTest::isValid() const
{
    bool okMeta = true;
    foreach(auto key, m_outputKeyList)
    {
      if(!hasMetaData(key))
      {
         qDebug() << "ERROR: test missing meta data" << key;
         okMeta = false;
         break;
       }
    }
    bool okTest = 0 < getNumberOfMeasurements();
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

QJsonObject ECGTest::toJsonObject() const
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
