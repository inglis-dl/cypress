#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QSerialPort>
#include <QSerialPortInfo>

#include "ManagerBase.h"
#include "../data/Measurement.h"

class SerialPortManager : public ManagerBase
{
    Q_OBJECT

    Q_PROPERTY(QString deviceName MEMBER m_deviceName NOTIFY deviceNameChanged)

public:
    explicit SerialPortManager(QObject *parent = Q_NULLPTR);

    bool isDefined(const QString &) const;

    QJsonObject toJsonObject() const override;

public slots:

    // what the manager does in response to the main application
    // window invoking its run method
    //
    void start() override;

    // connect to the serial port
    // opens the serial port with required parametere (baud rate etc.)
    // connects the port readyRead signal to the readDevice slot
    // emits canMeasure signal if the port is open
    //
    virtual void connectDevice();

    // disconnect from the serial port
    //
    void disconnectDevice();

    // select a device (serial port) by name
    // checks of the named port is in the list of scanned devices
    // and calls setDevice
    //
    void selectDevice(const QString &);

private slots:

    // send write request over RS232 to retrieve data from the audiometer
    //
    virtual void writeDevice() = 0;

    // retrieve data from the audiometer over RS232
    // emits canWrite signal if the test data is valid
    //
    virtual void readDevice() = 0;

    // set the serial port
    //
    void setDevice(const QSerialPortInfo &);

signals:

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
    void deviceSelected(const QString &);

    // port ready to connect
    // (update GUI enable connect button, disable disconnect button)
    //
    void canConnectDevice();

    void deviceNameChanged(const QString &);

protected:

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

    // device data is separate from test data
    Measurement m_deviceData;

    // list of available serial ports
    QMap<QString,QSerialPortInfo> m_deviceList;

    // the currently selected serial port
    QSerialPort m_port;
    QString m_deviceName;

    // serial port output receive buffer
    QByteArray m_buffer;

    // serial port input signal
    QByteArray m_request;
};

#endif // SERIALPORTMANAGER_H
