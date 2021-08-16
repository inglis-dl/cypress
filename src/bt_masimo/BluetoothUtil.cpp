#include "BluetoothUtil.h"

#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QLowEnergyCharacteristic>

#include <QByteArray>
#include <QString>


QString BluetoothUtil::uuidToString(const QBluetoothUuid &uuid)
{
    bool success = false;
    quint16 result16 = uuid.toUInt16(&success);
    if (success)
        return QStringLiteral("0x") + QString::number(result16, 16);

    quint32 result32 = uuid.toUInt32(&success);
    if (success)
        return QStringLiteral("0x") + QString::number(result32, 16);

    return uuid.toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
}

QString BluetoothUtil::valueToString(const QByteArray &a)
{
    // Show raw string first and hex value below
    QString result;
    if (a.isEmpty())
    {
        result = QStringLiteral("<none>");
        return result;
    }

    result = a;
    result += QLatin1Char('\n');
    result += a.toHex();

    return result;
}

QString BluetoothUtil::permissionToString(const QLowEnergyCharacteristic &c)
{
    QString properties = "( ";
    uint permission = c.properties();
    if (permission & QLowEnergyCharacteristic::Read)
        properties += QStringLiteral(" Read");
    if (permission & QLowEnergyCharacteristic::Write)
        properties += QStringLiteral(" Write");
    if (permission & QLowEnergyCharacteristic::Notify)
        properties += QStringLiteral(" Notify");
    if (permission & QLowEnergyCharacteristic::Indicate)
        properties += QStringLiteral(" Indicate");
    if (permission & QLowEnergyCharacteristic::ExtendedProperty)
        properties += QStringLiteral(" ExtendedProperty");
    if (permission & QLowEnergyCharacteristic::Broadcasting)
        properties += QStringLiteral(" Broadcast");
    if (permission & QLowEnergyCharacteristic::WriteNoResponse)
        properties += QStringLiteral(" WriteNoResp");
    if (permission & QLowEnergyCharacteristic::WriteSigned)
        properties += QStringLiteral(" WriteSigned");
    properties += " )";
    return properties;
}

