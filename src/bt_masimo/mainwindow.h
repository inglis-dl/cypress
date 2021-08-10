#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QLowEnergyController>
#include <QDir>

QT_FORWARD_DECLARE_CLASS(QBluetoothDeviceInfo)
QT_FORWARD_DECLARE_CLASS(QBluetoothLocalDevice)
QT_FORWARD_DECLARE_CLASS(QListWidgetItem)

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

    void scanDevices();

    // QBluetoothDeviceDiscoveryAgent signal receptors
    //
    // The agent searches for paired devices cached by the OS's BT local device
    // If the BT thermometer (peripheral) was previously paired to the client
    // create and activate a QLowEnergyController to negotiate communication
    //
    void deviceDiscovered(const QBluetoothDeviceInfo &info);
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void deviceDiscoveryFinished();

    void deviceSelected(QListWidgetItem*);

    // QLowEnergyController signal receptors
    //
    // The controller searches available services offered by the peripheral
    // If the Health Thermometer service is discovered, the controller
    // creates a QLowEnergyService to write and read temperature data
    //
    void serviceDiscovered(const QBluetoothUuid &uuid);
    void serviceDiscoveryComplete();
    void serviceScanError(QLowEnergyController::Error error);
    void discoverServices();

    // QLowEnergyService signal receptors
    //
    // The service negotiates the temperature data request from the peripheral
    //
    void serviceDetailsState(QLowEnergyService::ServiceState newState);
    void updateTemperatureValue(const QLowEnergyCharacteristic &c, const QByteArray &a);

    // Write the measurement data (temperature, datetime, barcode, device) in json formal to file
    //
    void writeMeasurement();

    void discoverPeripheralServices(const QBluetoothDeviceInfo &info);

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

    void connectToController(const QBluetoothDeviceInfo &info);

    bool m_foundThermoService = false;
    bool m_foundInfoService = false;
    QDir m_appDir;
    QMap<QString,QVariant> m_measurementData;
    QMap<QString,QVariant> m_deviceData;
    QString m_peripheralMAC;
    QMap<QString,QBluetoothDeviceInfo> m_deviceList;
};
#endif // MAINWINDOW_H
