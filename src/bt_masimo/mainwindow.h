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

extern const QString peripheralMAC;

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
        this->appDir = QDir(path);
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
    void deviceDiscoveryComplete();
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
    void deviceDisconnected();
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

    QBluetoothDeviceInfo *peripheral = nullptr;
    QBluetoothLocalDevice *client = nullptr;
    QBluetoothDeviceDiscoveryAgent *agent = nullptr;
    QLowEnergyController *controller = nullptr;

    // Read and wrtie storage for client and peripheral addresses in .ini file
    //
    void readSettings();
    void writeSettings();

    bool foundThermometer = false;
    QDir appDir;
    QMap<QString,QVariant> measurement;
};
#endif // MAINWINDOW_H
