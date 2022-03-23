#include "ECGMeasurement.h"

#include <QDebug>
#include <QDomElement>

bool ECGMeasurement::isValid() const
{
    QStringList keys = {
        "diagnosis",
        "ventricular_rate",
        "PQ_interval",
        "P_duration",
        "QRS_duration",
        "QT_interval",
        "QTC_interval",
        "RR_interval",
        "PP_interval",
        "P_axis",
        "R_axis",
        "T_axis",
        "QT_dispersion",
        "QRS_number",
        "P_onset",
        "P_offset",
        "Q_onset",
        "Q_offset",
        "T_offset"
    };
    bool ok = true;
    foreach(auto key, keys)
    {
      if(!hasAttribute(key))
      {
        qDebug() << "missing measurement attribute" << key;
        ok = false;
        break;
      }
    }
    return ok;
}

void ECGMeasurement::fromDomNode(const QDomNode &node)
{
  readRestingECGMeasurements(node);
  readInterpretation(node);
}

void ECGMeasurement::readInterpretation(const QDomNode& node)
{
    QDomElement child = node.firstChildElement("Diagnosis");
    if(child.isNull())
    {
        return;
    }
    QDomNodeList list = child.elementsByTagName("DiagnosisText");
    if(list.isEmpty())
    {
        return;
    }
    QStringList arr;
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
        return;
    }
    if(1 < arr.size())
    {
        setAttribute("diagnosis",arr);
    }
    else
    {
        setAttribute("diagnosis",arr.first());
    }
}

void ECGMeasurement::readRestingECGMeasurements(const QDomNode& node)
{
    QDomNodeList list = node.childNodes();
    if(list.isEmpty())
    {
        return;
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
              setAttribute(map[tag],"no" ==s ? false : true);
            else
            {
                if(elem.hasAttribute("units"))
                {
                   setAttribute(map[tag],s.toInt(),elem.attribute("units"));
                }
                else
                   setAttribute(map[tag],s);
            }
          }
       }
    }
}

QString ECGMeasurement::toString() const
{
    QString s;
    if(isValid())
    {
        //TODO: make a string
    }
    return s;
}

QDebug operator<<(QDebug dbg, const ECGMeasurement &item)
{
    const QString s = item.toString();
    if (s.isEmpty())
        dbg.nospace() << "ECG Measurement()";
    else
        dbg.nospace() << "ECG Measurement(" << s << " ...)";
    return dbg.maybeSpace();
}
