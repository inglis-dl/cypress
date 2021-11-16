#ifndef AUDIOMETERMANAGER_H
#define AUDIOMETERMANAGER_H

#include <QObject>
#include <QMap>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStandardItemModel>
#include <QVariant>

#include "../../data/AudiometerTest.h"

QT_FORWARD_DECLARE_CLASS(QSettings)

class AudiometerManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString deviceName MEMBER m_deviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(bool verbose READ isVerbose WRITE setVerbose)
    Q_PROPERTY(QString mode READ mode WRITE setMode)

public:
    explicit AudiometerManager(QObject *parent = nullptr);

    // check if any devices (serial ports) are available
    //
    bool devicesAvailable() const;

    // scan for available devices (serial ports)
    // emits scanning signal at start
    // populates a list of devices using serial port name as key
    // emits discovered signal with the serial port name when a port is discovered
    // if the ini stored port is found
    //   setDevice
    // else
    //   emits canSelect signal
    //
    void scanDevices();

    // select a device (serial port) by name
    // checks of the named port is in the list of scanned devices
    // and calls setDevice
    //
    void selectDevice(const QString &);

    void loadSettings(const QSettings &);
    void saveSettings(QSettings*) const;

    void setVerbose(const bool& verbose) { m_verbose = verbose; }
    bool isVerbose() const { return m_verbose; }

    void setMode(const QString& mode) { m_mode = mode; }
    QString mode() { return m_mode; }

    QJsonObject toJsonObject() const;

    void buildModel(QStandardItemModel *);

public slots:

    // connect to the serial port
    // opens the serial port with required parametere (baud rate etc.)
    // connects the port readyRead signal to the readDevice slot
    // emits canMeasure signal if the port is open
    //
    void connectDevice();

    // disconnect from the serial port
    //
    void disconnectDevice();

    // send write request over RS232 to retrieve data from the audiometer
    //
    void writeDevice();

private slots:

    // retrieve data from the audiometer over RS232
    // emits canWrite signal if the test data is valid
    //
    void readDevice();

    // set the serial port
    //
    void setDevice(const QSerialPortInfo &);

signals:

    // the underlying test data has changed
    //
    void dataChanged();

    // port scanning started
    // (update GUI status)
    //
    void scanningDevices();

    // a single port was discovered during the scan process
    // (update GUI list of ports)
    //
    void deviceDiscovered(const QString &);

    // a list of scanned port devices is avaiable for selection
    // (update GUI to prompt for user to select a port)
    //
    void canSelectDevice();

    // a port was selected from the list of discovered ports
    //
    void deviceSelected();

    // port ready to connect
    // (update GUI enable connect button)
    //
    void canConnectDevice();

    // valid test completed and ready to write to output
    // (update GUI enable write button and update the results display)
    //
    void canWrite();

    // ready to measure and receive data
    // (update GUI enable measure button)
    //
    void canMeasure();

    void deviceNameChanged(const QString &);

private:

    bool hasEndCode(const QByteArray &);

    // keep device data separate from test data
    //
    MeasurementBase m_deviceData;

    AudiometerTest m_test;

    QMap<QString,QSerialPortInfo> m_deviceList;
    QSerialPort m_port;
    QString m_deviceName;

    bool m_verbose;

    // mode of operation
    // - "simulate" - no devices are connected and the manager
    // responds to the UI signals and slots as though in live mode with valid
    // device and test data
    // - "live" - production mode
    //
    QString m_mode;

    QByteArray m_buffer;

    void clearData();
};

#endif // AUDIOMETERMANAGER_H
