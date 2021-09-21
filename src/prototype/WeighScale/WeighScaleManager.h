#ifndef WEIGHSCALEMANAGER_H
#define WEIGHSCALEMANAGER_H

#include <QObject>
#include <QMap>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVariant>

#include "../../data/WeightMeasurement.h"

QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(QJsonObject)

class WeighScaleManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString portName MEMBER m_portName NOTIFY portNameChanged)
    Q_PROPERTY(bool verbose READ isVerbose WRITE setVerbose)
    Q_PROPERTY(quint8 numberOfMeasurements READ getNumberOfMeasurements WRITE setNumberOfMeasurements)

public:
    explicit WeighScaleManager(QObject *parent = nullptr);

    bool localPortsEnabled() const;
    bool isDefined(const QString &) const;

    void selectDevice(const QString &);
    void scanDevices();

    void loadSettings(const QSettings &);
    void saveSettings(QSettings*) const;

    void setVerbose(const bool& verbose) { m_verbose = verbose; }
    bool isVerbose() const { return m_verbose; }

    void setNumberOfMeasurements(const quint8& n){ m_numberOfMeasurements = 0<n?n:1;}
    quint8 getNumberOfMeasurements() const {return m_numberOfMeasurements;}

    QJsonObject toJsonObject() const;

public slots:

    // zero the scale
    void zero();

    // retrieve the last weight measurement from the scale
    void measure();

signals:

    // port scanning started
    void scanning();

    // a single port was discovered during the scan process
    void discovered(const QString &);

    // a list of scanned port devices is avaiable for selection
    void canSelect();

    // a port was selected from the list of discovered ports
    void selected();

    // port ready to connect
    void canConnect();

    // data is available to write
    void measured(const QString &);

    void canWrite();

    // ready to measure after zeroing out the scale
    void canMeasure();

    void portNameChanged(const QString &);


private slots:

    void setDevice(const QSerialPortInfo &);

private:

    QList<WeightMeasurement> m_measurementData;
    QMap<QString,QVariant> m_deviceData;

    QMap<QString,QSerialPortInfo> m_deviceList;
    QSerialPort m_port;
    QString m_portName;

    bool m_verbose;
    quint8 m_numberOfMeasurements;

    void clearData();
};

#endif // WEIGHSCALEMANAGER_H
