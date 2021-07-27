#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QLowEnergyController>

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

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &info);
    void deviceDiscoveryComplete();
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error);

    void serviceDiscovered(const QBluetoothUuid &service);
    void serviceDiscoveryComplete();
    void serviceScanError(QLowEnergyController::Error error);
    void deviceDisconnected();
    void discoverServices();
    void serviceDetailsState(QLowEnergyService::ServiceState newState);

    void updateTemperatureValue(const QLowEnergyCharacteristic &c, const QByteArray& a);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor& d, const QByteArray& a);

private:
    Ui::MainWindow *ui;

    QBluetoothDeviceInfo *peripheral = nullptr;
    QBluetoothLocalDevice *client = nullptr;
    QBluetoothDeviceDiscoveryAgent *agent = nullptr;
    QLowEnergyController *controller = nullptr;
    void readSettings();
    void writeSettings();

    bool foundThermometer = false;
};
#endif // MAINWINDOW_H
