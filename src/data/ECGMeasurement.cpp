#include "ECGMeasurement.h"

#include <QDebug>
#include <QDomElement>
#include <QRandomGenerator>

bool ECGMeasurement::isValid() const
{
    QStringList keys = {
        "diagnosis",
        "ventricular_rate",
        "pq_interval",
        "p_duration",
        "qrs_duration",
        "qt_interval",
        "qtc_interval",
        "rr_interval",
        "pp_interval",
        "p_axis",
        "r_axis",
        "t_axis",
        "qrs_number",
        "p_onset",
        "p_offset",
        "q_onset",
        "q_offset",
        "t_offset"
    };
    bool ok = true;
    foreach(const auto key, keys)
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
      return;

    QDomNodeList list = child.elementsByTagName("DiagnosisText");
    if(list.isEmpty())
      return;

    QStringList arr;
    for(int i=0; i<list.size(); i++)
    {
      QDomElement elem = list.item(i).toElement();
      if(!elem.text().isEmpty())
        arr.append(elem.text());
    }
    if(arr.isEmpty())
      return;

    if(1 < arr.size())
      setAttribute("diagnosis",arr);
    else
      setAttribute("diagnosis",arr.first());
}

void ECGMeasurement::readRestingECGMeasurements(const QDomNode& node)
{
    QDomNodeList list = node.childNodes();
    if(list.isEmpty())
    {
      return;
    }
    QMap<QString,QString> map = {
        {"VentricularRate", "ventricular_rate"},
        {"PQInterval", "pq_interval"},
        {"PDuration", "p_duration"},
        {"QRSDuration", "qrs_duration"},
        {"QTInterval", "qt_interval"},
        {"QTCInterval", "qtc_interval"},
        {"RRInterval", "rr_interval"},
        {"PPInterval", "pp_interval"},
        {"SokolovLVHIndex", "sokolov_lvh_index"}, // not used
        {"PAxis", "p_axis"},
        {"RAxis", "r_axis"},
        {"TAxis", "t_axis"},
        {"QTDispersion", "qt_dispersion"},  // not used
        {"QTDispersionBazett", "qt_dispersion_bazett"}, //not used
        {"QRSNum", "qrs_number"},
        {"POnset", "p_onset"},
        {"POffset", "p_offset"},
        {"QOnset", "q_onset"},
        {"QOffset", "q_offset"},
        {"TOffset", "t_offset"}
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
              setAttribute(map[tag],("no" == s ? false : true));
            else
            {
              if(elem.hasAttribute("units"))
                setAttribute(map[tag],s.toInt(),elem.attribute("units"));
              else
                setAttribute(map[tag],s);
            }
          }
       }
    }
}

void ECGMeasurement::simulate()
{
    setAttribute("diagnosis","Normal sinus rhythm Normal ECG");

    int ival = QRandomGenerator::global()->bounded(32,107);
    setAttribute("ventricular_rate",ival,"BPM");

    ival = QRandomGenerator::global()->bounded(64,292);
    setAttribute("pq_interval",ival,"ms");

    ival = QRandomGenerator::global()->bounded(40,139);
    setAttribute("p_duration",ival,"ms");

    ival = QRandomGenerator::global()->bounded(52,172);
    setAttribute("qrs_duration",ival,"ms");

    ival = QRandomGenerator::global()->bounded(306,522);
    setAttribute("qt_interval",ival,"ms");

    ival = QRandomGenerator::global()->bounded(346,520);
    setAttribute("qtc_interval",ival,"ms");

    ival = QRandomGenerator::global()->bounded(406,1451);
    setAttribute("rr_interval",ival,"ms");

    ival = QRandomGenerator::global()->bounded(0,3000);
    setAttribute("pp_interval",ival,"ms");

    ival = QRandomGenerator::global()->bounded(-40,135);
    setAttribute("p_axis",ival,"degrees");

    ival = QRandomGenerator::global()->bounded(-90,162);
    setAttribute("r_axis",ival,"degrees");

    ival = QRandomGenerator::global()->bounded(-90,162);
    setAttribute("t_axis",ival,"degrees");

    ival = QRandomGenerator::global()->bounded(5,19);
    setAttribute("qrs_number",ival);

    ival = QRandomGenerator::global()->bounded(156,360);
    setAttribute("p_onset",ival,"ms");

    ival = QRandomGenerator::global()->bounded(216,444);
    setAttribute("p_offset",ival,"ms");

    ival = QRandomGenerator::global()->bounded(372,462);
    setAttribute("q_onset",ival,"ms");

    ival = QRandomGenerator::global()->bounded(498,594);
    setAttribute("q_offset",ival,"ms");

    ival = QRandomGenerator::global()->bounded(746,962);
    setAttribute("t_offset",ival,"ms");
}

QString ECGMeasurement::toString() const
{
    QString str;
    if(isValid())
    {
        //TODO: make a string
    }
    return str;
}

QDebug operator<<(QDebug dbg, const ECGMeasurement& item)
{
    const QString str = item.toString();
    if(str.isEmpty())
      dbg.nospace() << "ECG Measurement()";
    else
      dbg.nospace() << "ECG Measurement(" << str << " ...)";
    return dbg.maybeSpace();
}
