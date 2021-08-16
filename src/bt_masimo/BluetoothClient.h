#ifndef BLUETOOTHCLIENT_H
#define BLUETOOTHCLIENT_H

#include <QObject>

class BluetoothClient : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothClient(QObject *parent = nullptr);

signals:

};

#endif // BLUETOOTHCLIENT_H
