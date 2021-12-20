#include "CDTTMeasurement.h"

#include <QDebug>
#include <QRandomGenerator>

/**
 * sample input
 1,1,5,4,1,5,4
 * 
 *
 */

void CDTTMeasurement::fromString(const QString &commaSeperatedLine)
{
    // parse the comma deliminated string
    QStringList lineSplit = commaSeperatedLine.split(",");
    if(7 == lineSplit.size())
    {
        setCharacteristic("measurement number", lineSplit.at(0).toUInt());
        setCharacteristic("stimulus digit 1", lineSplit.at(1).toUInt());
        setCharacteristic("stimulus digit 2", lineSplit.at(2).toUInt());
        setCharacteristic("stimulus digit 3", lineSplit.at(3).toUInt());
        setCharacteristic("response digit 1", lineSplit.at(4).toUInt());
        setCharacteristic("response digit 2", lineSplit.at(5).toUInt());
        setCharacteristic("response digit 3", lineSplit.at(6).toUInt());
    }
}

bool CDTTMeasurement::isValid() const
{
    bool ok =
            hasCharacteristic("measurement number") &&
            hasCharacteristic("stimulus digit 1") &&
            hasCharacteristic("stimulus digit 2") &&
            hasCharacteristic("stimulus digit 3") &&
            hasCharacteristic("response digit 1") && 
            hasCharacteristic("response digit 2") &&
            hasCharacteristic("response digit 3");
    return ok;
}

QString CDTTMeasurement::toString() const
{
  QString measurementStr;
  if(isValid())
  {
    measurementStr = QString("#%0. Stimulus: %1%2%3 Response: %4%5%6").arg(
        getCharacteristic("measurement number").toString(),
        getCharacteristic("stimulus digit 1").toString(),
        getCharacteristic("stimulus digit 2").toString(),
        getCharacteristic("stimulus digit 3").toString(),
        getCharacteristic("response digit 1").toString(),
        getCharacteristic("response digit 2").toString(),
        getCharacteristic("response digit 3").toString()
    );
  }
  return measurementStr;
}

CDTTMeasurement CDTTMeasurement::simulate()
{
    CDTTMeasurement simulatedMeasurement;

    int measurementNumber = QRandomGenerator::global()->bounded(1, 24);
    int stimulusDigit1 = QRandomGenerator::global()->bounded(1, 9);
    int stimulusDigit2 = QRandomGenerator::global()->bounded(1, 9);
    int stimulusDigit3 = QRandomGenerator::global()->bounded(1, 9);
    int responseDigit1 = QRandomGenerator::global()->bounded(1, 10) > 1 ? stimulusDigit1 : QRandomGenerator::global()->bounded(1, 9);
    int responseDigit2 = QRandomGenerator::global()->bounded(1, 10) > 1 ? stimulusDigit2 : QRandomGenerator::global()->bounded(1, 9);
    int responseDigit3 = QRandomGenerator::global()->bounded(1, 10) > 1 ? stimulusDigit3 : QRandomGenerator::global()->bounded(1, 9);

    simulatedMeasurement.setCharacteristic("measurement number", measurementNumber);
    simulatedMeasurement.setCharacteristic("stimulus digit 1", stimulusDigit1);
    simulatedMeasurement.setCharacteristic("stimulus digit 2", stimulusDigit2);
    simulatedMeasurement.setCharacteristic("stimulus digit 3", stimulusDigit3);
    simulatedMeasurement.setCharacteristic("response digit 1", responseDigit1);
    simulatedMeasurement.setCharacteristic("response digit 2", responseDigit2);
    simulatedMeasurement.setCharacteristic("response digit 3", responseDigit3);

    return simulatedMeasurement;
}

QDebug operator<<(QDebug dbg, const CDTTMeasurement &item)
{
    const QString measurementStr = item.toString();
    if (measurementStr.isEmpty())
        dbg.nospace() << "CDTTT Measurement()";
    else
        dbg.nospace() << "CDTTT Measurement(" << measurementStr << " ...)";
    return dbg.maybeSpace();
}
