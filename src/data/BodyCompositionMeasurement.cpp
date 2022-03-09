#include "BodyCompositionMeasurement.h"

#include <QDebug>

QStringList BodyCompositionMeasurement::variableList = BodyCompositionMeasurement::initVariableList();

QStringList BodyCompositionMeasurement::initVariableList()
{
  QStringList list;
  list <<
     "weight" <<
     "impedance" <<
     "percent_fat" <<
     "fat_mass" <<
     "fat_free_mass" <<
     "total_body_water" <<
     "body_mass_index" <<
     "basal_metabolic_rate";

  return list;
}

bool BodyCompositionMeasurement::isValid() const
{
  bool ok = true;
  foreach(auto key, BodyCompositionMeasurement::variableList)
  {
     if(!hasAttribute(key) || getAttribute(key).isNull())
     {
        ok = false;
        break;
     }
  }
  return ok;
}

QDebug operator<<(QDebug dbg, const BodyCompositionMeasurement &item)
{
    const QString s = item.toString();
    if (s.isEmpty())
        dbg.nospace() << "Body Composition Measurement()";
    else
        dbg.nospace() << "Body Composition Measurement(" << s << " ...)";
    return dbg.maybeSpace();
}
