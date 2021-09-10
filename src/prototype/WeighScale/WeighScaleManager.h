#ifndef WEIGHSCALEMANAGER_H
#define WEIGHSCALEMANAGER_H

#include <QObject>
#include <QMap>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVariant>

QT_FORWARD_DECLARE_CLASS(QSettings)

class WeighScaleManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString portName MEMBER m_portName NOTIFY portNameChanged)
    Q_PROPERTY(QString weight MEMBER m_weight NOTIFY weightChanged)
    Q_PROPERTY(QString datetime MEMBER m_datetime NOTIFY datetimeChanged)
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

    const QMap<QString,QVariant>&  getMeasurementData(){return m_measurementData;}
    const QMap<QString,QVariant>&  getDeviceData(){return m_deviceData;}

    QMap<QString,QVariant> getData(){
        QMap<QString,QVariant> map(m_measurementData);
        QMap<QString,QVariant>::const_iterator it = m_deviceData.constBegin();
        while(it != m_deviceData.constEnd())
        {
          map[it.key()] = it.value();
          ++it;
        }
        return map;
    }

public slots:

    //void connectToPort();
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
    void canWrite();

    void canMeasure();

    void portNameChanged(const QString &);
    void weightChanged(const QString &);
    void datetimeChanged(const QString &);

private slots:

    void setDevice(const QSerialPortInfo &);

private:

    QString m_portName;
    QMap<QString,QVariant> m_measurementData;
    QMap<QString,QVariant> m_deviceData;

    QMap<QString,QSerialPortInfo> m_deviceList;
    QSerialPortInfo m_portInfo;
    QSerialPort m_port;

    QString m_weight;
    QString m_datetime;
    bool m_verbose;

    void clearData();

};

#endif // WEIGHSCALEMANAGER_H
