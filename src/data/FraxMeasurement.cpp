#include "FraxMeasurement.h"

#include <QDebug>

void FraxMeasurement::fromString(const QString &inputStr)
{
    // parse the comma deliminated string
    QStringList commaSplit = inputStr.split(",");
    if (17 == commaSplit.size())
    {
        setCharacteristic("val1", commaSplit.at(0));
        setCharacteristic("val2", commaSplit.at(1).toDouble());
        setCharacteristic("val3", commaSplit.at(2).toDouble());
        setCharacteristic("val4", commaSplit.at(3).toDouble());
        setCharacteristic("val5", commaSplit.at(4).toDouble());
        setCharacteristic("val6", commaSplit.at(5).toDouble());
        setCharacteristic("val7", commaSplit.at(6).toDouble());
        setCharacteristic("val8", commaSplit.at(7).toDouble());
        setCharacteristic("val9", commaSplit.at(8).toDouble());
        setCharacteristic("val10", commaSplit.at(9).toDouble());
        setCharacteristic("val11", commaSplit.at(10).toDouble());
        setCharacteristic("val12", commaSplit.at(11).toDouble());
        setCharacteristic("dxaHipTScore", commaSplit.at(12).toDouble());
        setCharacteristic("output1", commaSplit.at(13).toDouble());
        setCharacteristic("output2", commaSplit.at(14).toDouble());
        setCharacteristic("output3", commaSplit.at(15).toDouble());
        setCharacteristic("output4", commaSplit.at(16).toDouble());
    }
}

bool FraxMeasurement::isValid() const
{
    bool ok =
        hasCharacteristic("val1") &&
        hasCharacteristic("val2") &&
        hasCharacteristic("val3") &&
        hasCharacteristic("val4") &&
        hasCharacteristic("val5") &&
        hasCharacteristic("val6") &&
        hasCharacteristic("val7") &&
        hasCharacteristic("val8") &&
        hasCharacteristic("val9") &&
        hasCharacteristic("val10") &&
        hasCharacteristic("val11") &&
        hasCharacteristic("val12") &&
        hasCharacteristic("dxaHipTScore") &&
        hasCharacteristic("output1") &&
        hasCharacteristic("output2") &&
        hasCharacteristic("output3") &&
        hasCharacteristic("output4");
    return ok;
}

QString FraxMeasurement::toString() const
{
    QString displayString;
    if (isValid()) {
        displayString = QString("Out1 (%1), Out2 (%2), Out3 (%3), Out4 (%4)")
            .arg(
                getCharacteristic("output1").toString(),
                getCharacteristic("output2").toString(),
                getCharacteristic("output3").toString(),
                getCharacteristic("output4").toString()
            );
    }
	return displayString;
}

QDebug operator<<(QDebug dbg, const FraxMeasurement &item)
{
    const QString s = item.toString();
    if (s.isEmpty())
        dbg.nospace() << "Frax Measurement()";
    else
        dbg.nospace() << "Frax Measurement(" << s << " ...)";
    return dbg.maybeSpace();
}
