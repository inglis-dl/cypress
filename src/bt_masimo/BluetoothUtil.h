#ifndef BLUETOOTHUTIL_H
#define BLUETOOTHUTIL_H

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QBluetoothUuid)
QT_FORWARD_DECLARE_CLASS(QLowEnergyCharacteristic)

class BluetoothUtil : public QObject
{
    Q_OBJECT
public:
    static QString uuidToString(const QBluetoothUuid&);
    static QString valueToString(const QByteArray&);
    static QString permissionToString(const QLowEnergyCharacteristic&);
};

#endif // BLUETOOTHUTIL_H
