#include "FraxMeasurement.h"

#include <QDebug>

void FraxMeasurement::fromString(const QString &inputStr)
{
    // parse the comma deliminated string
    QStringList commaSplit = inputStr.split(",");
    if (17 == commaSplit.size())
    {
        setCharacteristic("val1", commaSplit.at(0));
        setCharacteristic("val2", commaSplit.at(1).toUInt());
        setCharacteristic("val3", commaSplit.at(2).toUInt());
        setCharacteristic("val4", commaSplit.at(3).toUInt());
        setCharacteristic("val5", commaSplit.at(4).toUInt());
        setCharacteristic("val6", commaSplit.at(5).toUInt());
        setCharacteristic("val7", commaSplit.at(6).toUInt());
        setCharacteristic("val8", commaSplit.at(7).toUInt());
        setCharacteristic("val9", commaSplit.at(8).toUInt());
        setCharacteristic("val10", commaSplit.at(9).toUInt());
        setCharacteristic("val11", commaSplit.at(10).toUInt());
        setCharacteristic("val12", commaSplit.at(11).toUInt());
        setCharacteristic("dxaHipTScore", commaSplit.at(12).toUInt());
        setCharacteristic("output1", commaSplit.at(13).toUInt());
        setCharacteristic("output2", commaSplit.at(14).toUInt());
        setCharacteristic("output3", commaSplit.at(15).toUInt());
        setCharacteristic("output4", commaSplit.at(16).toUInt());
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
        displayString = QString("Out 1: %1\nOut 2: %2\nOut 3: %3\nOut 4: %4\n")
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
