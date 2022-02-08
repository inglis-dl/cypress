#include "TonometerMeasurement.h"

#include <QDebug>

/**
 * possible values from the Reichert ORA tonometer db
 *
          Int("MeasureID");
          Int("MeasureNumber");
          Date("MeasureDate");
          Date("SessionDate");
          String("Eye");
          String("ORASerialNumber");
          String("ORASoftware");
          String("PCSoftware");
          Decimal("IOPG");
          Decimal("IOPCC");
          Decimal("CRF");
          Decimal("CCTAvg");
          Decimal("CCTLowest");
          Decimal("CCTSD");
          Decimal("CH");
          Decimal("TearFilmValue");
          String("Pressure");
          String("Applanation");
          Decimal("TimeIn");
          Decimal("TimeOut");
          String("Meds");
          String("Conditions");
          String("Notes1");
          String("Notes2");
          String("Notes3");
          Decimal("m_G2");
          Decimal("b_G2");
          Decimal("m_G3");
          Decimal("b_G3");
          Decimal("iop_cc_coef");
          Decimal("crf_coef");
          Decimal("m_ABC");
          Decimal("b_ABC");
          Decimal("b_PP");
          Boolean("BestWeighted");
          Decimal("QualityIndex");
          String("Indexes");
  *
  *
 */

bool TonometerMeasurement::isValid() const
{
    bool ok =
      hasCharacteristic("units") &&
      hasCharacteristic("name") &&
      hasCharacteristic("value") &&
      hasCharacteristic("side");
    return ok;
}

QString TonometerMeasurement::toString() const
{
    QStringList skip = {"applanation,pressure,indexes"};
    QString s;
    if(isValid())
    {
      if(skip.contains(getCharacteristic("name").toString()))
      {
        s = QString("%1 %2: %3...").arg(
          getCharacteristic("side").toString(),
          getCharacteristic("name").toString(),
          getCharacteristic("value").toString().left(10));
      }
      else
      {
        if(getCharacteristic("units").isNull())
        {
          s = QString("%1 %2: %3").arg(
            getCharacteristic("side").toString(),
            getCharacteristic("name").toString(),
            getCharacteristic("value").toString());
        }
        else
        {
          s = QString("%1 %2: %3(%4)").arg(
            getCharacteristic("side").toString(),
            getCharacteristic("name").toString(),
            getCharacteristic("value").toString(),
            getCharacteristic("units").toString());
        }
      }
    }
    return s;
}

QDebug operator<<(QDebug dbg, const TonometerMeasurement &item)
{
    const QString s = item.toString();
    if (s.isEmpty())
        dbg.nospace() << "Tonometer Measurement()";
    else
        dbg.nospace() << "Tonometer Measurement(" << s << " ...)";
    return dbg.maybeSpace();
}
