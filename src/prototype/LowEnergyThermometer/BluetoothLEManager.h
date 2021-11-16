#ifndef BLUETOOTHLEMANAGER_H
#define BLUETOOTHLEMANAGER_H

#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QBluetoothLocalDevice>

#include "../../managers/ManagerBase.h"
#include "../../data/TemperatureTest.h"

QT_FORWARD_DECLARE_CLASS(QBluetoothDeviceInfo)
QT_FORWARD_DECLARE_CLASS(QBluetoothUuid)
QT_FORWARD_DECLARE_CLASS(QLowEnergyService)

class BluetoothLEManager : public ManagerBase
{
    Q_OBJECT
    Q_PROPERTY(QString deviceName MEMBER m_deviceName NOTIFY deviceNameChanged)

public:
    explicit BluetoothLEManager(QObject *parent = nullptr);

    // the host supports Bluetooth Low Energy discovery
    //
    bool lowEnergyEnabled() const;

    // the local adapter is enabled
    //
    bool localDeviceEnabled() const;

    // Select a peripheral bluetooth lowe energy device by name.
    // checks if the name as a key in the key value pair list of scanned devices
    // and calls setDevice with the QBluetoothInfo value
    //
    void selectDevice(const QString &);

    // the host adapter is paired to the device (BTLE peripheral)
    //
    bool isPairedTo(const QString &) const;

    void loadSettings(const QSettings &) override;
    void saveSettings(QSettings*) const override;

    void buildModel(QStandardItemModel *) const override;

    QJsonObject toJsonObject() const override;

signals:

    // Scanning started
    // (update GUI status)
    //
    void scanningDevices();

    // A single device was discovered during the scan process
    // (update GUI list of ports)
    //
    void deviceDiscovered(const QString &);

    // A list of scanned devices is avaiable for selection
    // (update GUI to prompt for user to select a port)
    //
    void canSelectDevice();

    // A peripheral was selected from the list of discovered devices
    //
    void deviceSelected(const QString &);

    // Peripheral device amd local adapter ready to connect
    // (update GUI enable connect button)
    //
    void canConnectDevice();

    // Valid test completed (with 2 readings) and ready to write to output
    // (update GUI enable write button and update the results display)
    //
    void canWrite();

    // Ready to measure and receive data
    // (update GUI enable measure button)
    //
    void canMeasure();

    void deviceNameChanged(const QString &);

public slots:

    // Scan for available peripheral devices.
    // emits scanning signal at start
    // populates a list of devices using device name as key
    // emits discovered signal with the device name when a peripheral is discovered
    // if the ini stored device is found
    //   setDevice
    // else
    //   emits canSelect signal
    //
    void scanDevices();

    // Connect to the peripheral
    //
    void connectDevice();

    // Disconnect from the peripheral
    //
    void disconnectDevice();

    // Retrieve a measurement from the device
    //
    void measure();

private slots:

    // QBluetoothDeviceDiscoveryAgent signal receptors
    //
    // The agent searches for paired devices cached by the OS.
    // If the thermometer (peripheral) was previously paired to the client
    // create and activate a QLowEnergyController to negotiate communication.
    //
    void deviceDiscoveredInternal(const QBluetoothDeviceInfo &info);

    // Discovery agent finished or cancelled device discovery.
    //
    void discoveryCompleteInternal();

    // Set the bluetooth low energy peripheral
    //
    void setDevice(const QBluetoothDeviceInfo &info);

    // QLowEnergyController signal receptors
    //
    // The controller searches available services offered by the peripheral
    // If the Health Thermometer service is discovered, the controller
    // creates a QLowEnergyService to write the Indicate property
    // of the client characteristic and then receive temperature data changes
    //
    void serviceDiscovered(const QBluetoothUuid &uuid);
    void serviceDiscoveryComplete();

    // QLowEnergyService signal receptors
    //
    // The service negotiates the temperature and device info data transfer from the peripheral.
    // Both the device information and the temperature services must first
    // be discovered before data requests are made.
    //
    void infoServiceStateChanged(QLowEnergyService::ServiceState state);
    void thermoServiceStateChanged(QLowEnergyService::ServiceState state);

    // Respond to read requests and indications
    //
    void updateTemperatureData(const QLowEnergyCharacteristic &c, const QByteArray &a);
    void updateInfoData(const QLowEnergyCharacteristic &c, const QByteArray &a);

 private:
    QScopedPointer<QBluetoothDeviceInfo> m_peripheral;
    QScopedPointer<QBluetoothLocalDevice> m_localDevice;
    QScopedPointer<QBluetoothDeviceDiscoveryAgent> m_agent;
    QScopedPointer<QLowEnergyController> m_controller;
    QScopedPointer<QLowEnergyService> m_thermo_service;
    QScopedPointer<QLowEnergyService> m_info_service;

    void connectToController(const QBluetoothDeviceInfo &info);

    void setLocalDevice(const QString &);

    MeasurementBase m_deviceData;

    TemperatureTest m_test;

    QMap<QString,QBluetoothDeviceInfo> m_deviceList;

    QString m_deviceName;

    void clearData() override;
};

#endif // BLUETOOTHLEMANAGER_H
