#ifndef WEIGHSCALEMANAGER_H
#define WEIGHSCALEMANAGER_H

#include <QObject>
#include <QMap>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVariant>

#include "Measurement.h"

QT_FORWARD_DECLARE_CLASS(QSettings)

class WeighScaleManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString portName MEMBER m_portName NOTIFY portNameChanged)
    Q_PROPERTY(bool verbose READ isVerbose WRITE setVerbose)

public:
    explicit WeighScaleManager(QObject *parent = nullptr);

    bool localPortsEnabled();
    bool isDefined(const QString &);

    void selectDevice(const QString &);
    void scanDevices();

    void loadSettings(const QSettings &);
    void saveSettings(QSettings*);

    void setVerbose(const bool& verbose) { m_verbose = verbose; }
    bool isVerbose(){ return m_verbose; }

    const QList<Measurement>&  getMeasurementData(){return m_measurementData;}
    const QMap<QString,QVariant>&  getDeviceData(){return m_deviceData;}

    /*
    QMap<Measurement> getData(){
        QList<QString,QVariant> map(m_measurementData);
        QMap<QString,QVariant>::const_iterator it = m_deviceData.constBegin();
        while(it != m_deviceData.constEnd())
        {
          map[it.key()] = it.value();
          ++it;
        }
        return map;
    }
    */

public slots:

    void zero();
    void measure();

signals:

    void scanning();
    void discovered(const QString &);

    // prompt the user to select the port from list of scanned port devices
    void canSelect();

    // a port was found in the list of discovered ports and serial was
    // selected and set to portSerial
    void selected();

    // port ready to connect
    void canConnect();

    // data is available to write
    void measured(const QString &);

    void canWrite();

    void canMeasure();

    void portNameChanged(const QString &);


private slots:

    void setDevice(const QSerialPortInfo &);

private:

    QString m_portName;
    QList<Measurement> m_measurementData;
    QMap<QString,QVariant> m_deviceData;

    QMap<QString,QSerialPortInfo> m_deviceList;
    QSerialPort m_port;

    bool m_verbose;

    void clearData();
};

#endif // WEIGHSCALEMANAGER_H
