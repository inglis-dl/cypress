#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QLowEnergyController>
#include <QDir>

QT_FORWARD_DECLARE_CLASS(QBluetoothDeviceInfo)
QT_FORWARD_DECLARE_CLASS(QBluetoothLocalDevice)

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class BLEInfo : public QObject
{
    Q_OBJECT
public:
    static QString uuidToString(const QBluetoothUuid&);
    static QString valueToString(const QByteArray&);
    static QString handleToString(const QLowEnergyHandle&);
    static QString permissionToString(const QLowEnergyCharacteristic&);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setApplicationDir(QString path) {
        this->m_appDir = QDir(path);
    }

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:

    // QBluetoothDeviceDiscoveryAgent signal receptors
    //
    // The agent searches for paired devices cached by the OS's BT local device
    // If the BT thermometer (peripheral) was previously paired to the client
    // create and activate a QLowEnergyController to negotiate communication
    //
    void deviceDiscovered(const QBluetoothDeviceInfo &info);
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error);

    // QLowEnergyController signal receptors
    //
    // The controller searches available services offered by the peripheral
    // If the Health Thermometer service is discovered, the controller
    // creates a QLowEnergyService to write and read temperature data
    //
    void serviceDiscovered(const QBluetoothUuid &service);
    void serviceDiscoveryComplete();
    void serviceScanError(QLowEnergyController::Error error);
    void discoverServices();

    // QLowEnergyService signal receptors
    //
    // The service negotiates the temperature data request from the peripheral
    //
    void serviceDetailsState(QLowEnergyService::ServiceState newState);
    void updateTemperatureValue(const QLowEnergyCharacteristic &c, const QByteArray& a);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor& d, const QByteArray& a);

    // Write the measurement data (temperature, datetime, barcode, device) in json formal to file
    //
    void writeMeasurement();

private:
    Ui::MainWindow *ui;

    QBluetoothDeviceInfo *m_peripheral = nullptr;
    QBluetoothLocalDevice *m_client = nullptr;
    QBluetoothDeviceDiscoveryAgent *m_agent = nullptr;
    QLowEnergyController *m_controller = nullptr;

    // Read and wrtie storage for client and peripheral addresses in .ini file
    //
    void readSettings();
    void writeSettings();

    bool foundThermometer = false;
    bool foundDeviceInfo = false;
    QDir m_appDir;
    QMap<QString,QVariant> m_measurementData;
    QMap<QString,QVariant> m_deviceData;
    QString m_peripheralMAC;
};
#endif // MAINWINDOW_H
