#include "CDTTMeasurement.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QRandomGenerator>

bool CDTTMeasurement::isValid() const
{
    bool ok =
      hasAttribute("trial") &&
      hasAttribute("stimulus") &&
      hasAttribute("response");
    return ok;
}

void CDTTMeasurement::simulate(const quint16& trial)
{
    QJsonArray stimulus;
    QJsonArray response;
    for(int i=0;i<3;i++)
    {
      stimulus.append(QJsonValue(QRandomGenerator::global()->bounded(0,9)));
      response.append(QJsonValue(QRandomGenerator::global()->bounded(0,9)));
    }
    QJsonObject obj;
    obj["trial"] = QJsonValue(trial);
    obj["stimulus"] = stimulus;
    obj["response"] = response;
    foreach(auto key, obj.keys())
    {
      setAttribute(key,obj[key].toVariant());
    }
}

QString CDTTMeasurement::toString() const
{
  QString str;
  if(isValid())
  {
    str = QString("trial #%1: stimulus [%2], response [%3]").arg(
          getAttribute("trial").toString(),
          getAttribute("stimulus").toString(),
          getAttribute("response").toString());
  }
  return str;
}

QDebug operator<<(QDebug dbg, const CDTTMeasurement &item)
{
    const QString measurementStr = item.toString();
    if(measurementStr.isEmpty())
      dbg.nospace() << "CDTT Measurement()";
    else
      dbg.nospace() << "CDTT Measurement(" << measurementStr << " ...)";
    return dbg.maybeSpace();
}
