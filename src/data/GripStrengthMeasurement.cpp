#include "GripStrengthMeasurement.h"

#include <QDebug>
#include <QJsonObject>
#include "../auxiliary/Utilities.h"

const q_stringMap GripStrengthMeasurement::trialMap =
{
  {"Side","trial_side"},
  {"Position","trial_rung_position"},
  {"Maximum","trial_max"},
  {"Average","exam_average"},
  {"CV","exam_cv"}
};

void GripStrengthMeasurement::fromRecord(const QJsonObject* record)
{
    foreach(const auto tag, trialMap.toStdMap()) {
        if (record->contains(tag.first)) {
            setAttribute(tag.second, record->value(tag.first).toVariant());
        }
    }
}

bool GripStrengthMeasurement::isValid() const
{
    bool okResult = true;
    foreach(const auto key, trialMap.values())
    {
        if (!hasAttribute(key))
        {
            qDebug() << "trial measurement missing trial attribute" << key;
            okResult = false;
            break;
        }
    }
    return okResult;
}

QString GripStrengthMeasurement::toString() const
{
    return QString("Side: %1 Rung: %2 Average: %3 Max: %4 CV: %5")
        .arg(getAttribute("trial_side").value().toString())
        .arg(getAttribute("trial_rung_position").value().toString())
        .arg(getAttribute("exam_average").value().toString())
        .arg(getAttribute("trial_max").value().toString())
        .arg(getAttribute("exam_cv").value().toString());
}

GripStrengthMeasurement GripStrengthMeasurement::simulate()
{
    GripStrengthMeasurement simulatedMeasurement;
    return simulatedMeasurement;
}

QDebug operator<<(QDebug dbg, const GripStrengthMeasurement& item)
{
    const QString measurementStr = item.toString();
    if (measurementStr.isEmpty())
        dbg.nospace() << "GripStrength Measurement()";
    else
        dbg.nospace() << "GripStrength Measurement(" << measurementStr << " ...)";
    return dbg.maybeSpace();
}
