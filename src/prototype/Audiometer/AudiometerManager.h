#ifndef AUDIOMETERMANAGER_H
#define AUDIOMETERMANAGER_H

#include <QObject>
#include <QMap>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVariant>

#include "../../data/HearingMeasurement.h"

QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(QJsonObject)

class AudiometerManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString portName MEMBER m_portName NOTIFY portNameChanged)
    Q_PROPERTY(bool verbose READ isVerbose WRITE setVerbose)
    Q_PROPERTY(quint8 numberOfMeasurements READ getNumberOfMeasurements WRITE setNumberOfMeasurements)

public:
    explicit AudiometerManager(QObject *parent = nullptr);

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


    enum class State {
        CONNECTED,
        DISCONNECTED
    };

public slots:

    void connectPort();

    void disconnectPort();

    // retrieve the last weight measurement from the scale
    void measure();

    void readData();
    void setDevice(const QSerialPortInfo &);

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

    void stateChanged(const State &);

private:

    static bool hasEndCode(const QByteArray &);

    QList<HearingMeasurement> m_measurementData;
    QMap<QString,QVariant> m_deviceData;

    QMap<QString,QSerialPortInfo> m_deviceList;
    QSerialPort m_port;
    QString m_portName;

    bool m_verbose;
    quint8 m_numberOfMeasurements;

    QByteArray m_buffer;

    void clearData();

    State m_state;
};

#endif // AUDIOMETERMANAGER_H
