#ifndef BLUETOOTHUTIL_H
#define BLUETOOTHUTIL_H

#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QLowEnergyCharacteristic>

class BluetoothUtil
{
public:
    static QString uuidToString(const QBluetoothUuid&);
    static QString valueToString(const QByteArray&);
    static QString permissionToString(const QLowEnergyCharacteristic&);
};

Q_DECLARE_METATYPE(BluetoothUtil);

#endif // BLUETOOTHUTIL_H
