#ifndef BLUETOOTHLEMANAGER_H
#define BLUETOOTHLEMANAGER_H

#include <QObject>

#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QBluetoothLocalDevice>

#include "../../data/TemperatureMeasurement.h"

QT_FORWARD_DECLARE_CLASS(QBluetoothDeviceInfo)
QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(QBluetoothUuid)
QT_FORWARD_DECLARE_CLASS(QLowEnergyService)

class BluetoothLEManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString peripheralMAC MEMBER m_peripheralMAC NOTIFY peripheralMACChanged)
    Q_PROPERTY(bool verbose READ isVerbose WRITE setVerbose)

public:
    explicit BluetoothLEManager(QObject *parent = nullptr);

    bool lowEnergyEnabled() const;
    bool localAdapterEnabled() const;
    void selectAdapter(const QString &);
    bool isPairedTo(const QString &) const;
    void selectDevice(const QString &);
    void scanDevices();
    void loadSettings(const QSettings &);
    void saveSettings(QSettings*);

    void setVerbose(const bool& verbose) { m_verbose = verbose; }
    bool isVerbose(){ return m_verbose; }

    QJsonObject toJsonObject() const;

signals:

    void peripheralMACChanged(const QString &);
    void temperatureChanged(const QString &);

    void scanning();

    // Update a UI containing a list of selectable labels
    //
    void discovered(const QString &); // label of discovered device

    // prompt the user to select the peripheral from list of scanned devices
    void canSelect();

    void selected(); // a device was found in the list of discovered devices and mac address was selected and set to peripheral mac

    // controller ready to connect
    void canConnect();

    // data is available to write
    void canWrite();

    // connected to peripheral
    void connected();

    void measured(const QString &);

public slots:
    void connectPeripheral();

private slots:

    // QBluetoothDeviceDiscoveryAgent signal receptors
    //
    // The agent searches for paired devices cached by the OS's BT local device
    // If the BT thermometer (peripheral) was previously paired to the client
    // create and activate a QLowEnergyController to negotiate communication
    //
    void deviceDiscovered(const QBluetoothDeviceInfo &info);
    void deviceDiscoveryComplete();
    void setDevice(const QBluetoothDeviceInfo &info);

    // QLowEnergyController signal receptors
    //
    // The controller searches available services offered by the peripheral
    // If the Health Thermometer service is discovered, the controller
    // creates a QLowEnergyService to write and read temperature data
    //
    void serviceDiscovered(const QBluetoothUuid &uuid);
    void serviceDiscoveryComplete();

    // QLowEnergyService signal receptors
    //
    // The service negotiates the temperature data request from the peripheral
    //
    void serviceStateChanged(QLowEnergyService::ServiceState state);

    void updateTemperatureData(const QLowEnergyCharacteristic &c, const QByteArray &a);
    void updateInfoData(const QLowEnergyCharacteristic &c, const QByteArray &a);

 private:

    QScopedPointer<QBluetoothDeviceInfo> m_peripheral;
    QScopedPointer<QBluetoothLocalDevice> m_client;
    QScopedPointer<QBluetoothDeviceDiscoveryAgent> m_agent;
    QScopedPointer<QLowEnergyController> m_controller;
    QScopedPointer<QLowEnergyService> m_thermo_service;
    QScopedPointer<QLowEnergyService> m_info_service;

    void connectToController(const QBluetoothDeviceInfo &info);

    bool m_verbose;
    bool m_foundThermoService;
    bool m_foundInfoService;

    QMap<QString,QVariant> m_deviceData;
    QList<TemperatureMeasurement> m_measurementData;
    QString m_peripheralMAC;

    QMap<QString,QBluetoothDeviceInfo> m_deviceList;

    void clearData();
    void disconnectPeripheral();
};

#endif // BLUETOOTHLEMANAGER_H
