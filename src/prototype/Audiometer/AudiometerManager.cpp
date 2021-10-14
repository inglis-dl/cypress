#include "AudiometerManager.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QSerialPortInfo>
#include <QSettings>
#include <QtMath>

AudiometerManager::AudiometerManager(QObject *parent) : QObject(parent),
    m_verbose(false),
    m_numberOfMeasurements(1),
    m_state(State::DISCONNECTED)
{
}

bool AudiometerManager::localPortsEnabled() const
{
    qDebug() << "checking available ports";
    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    bool result = list.empty();
    qDebug() << "got list result and returning";
    return !result;
}

void AudiometerManager::loadSettings(const QSettings &settings)
{
    QString portName = settings.value("client/port").toString();
    if(!portName.isEmpty())
    {
      setProperty("portName", portName);
      if(m_verbose)
          qDebug() << "using serial port " << m_portName << " from settings file";
    }
}

void AudiometerManager::saveSettings(QSettings *settings) const
{
    if(!m_portName.isEmpty())
    {
      settings->setValue("client/port",m_portName);
      if(m_verbose)
          qDebug() << "wrote serial port to settings file";
    }
}

void AudiometerManager::clearData()
{
    m_measurementData.clear();
    m_deviceData.clear();
}

void AudiometerManager::scanDevices()
{
    m_deviceList.clear();
    emit scanning();
    qDebug() << "start scanning for devices ....";

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        qDebug() << "in loop";
        if(m_verbose)
        {
          // get the device data
          if(info.hasProductIdentifier())
            qDebug() << "product id:" << QString::number(info.productIdentifier());
          if(info.hasVendorIdentifier())
            qDebug() << "vendor id:" << QString::number(info.vendorIdentifier());
          if(!info.manufacturer().isEmpty())
            qDebug() << "manufacturer:" << info.manufacturer();
          if(!info.portName().isEmpty())
            qDebug() << "port name:" << info.portName();
          if(!info.serialNumber().isEmpty())
            qDebug() << "serial number:" << info.serialNumber();
          if(!info.systemLocation().isEmpty())
            qDebug() << "system location:" << info.systemLocation();
          if(!info.description().isEmpty())
            qDebug() << "description:" << info.description();
        }
        QString label = QString("%1 %2").arg(info.portName(),info.description());
        if(!m_deviceList.contains(label))
        {
            m_deviceList.insert(label,info);
            emit discovered(label);
            qDebug() << "found device " << label;
        }
    }
    if(m_verbose)
        qDebug() << "Found " << QString::number(m_deviceList.count()) << " ports";

    // if we have a port from the ini file, check if it is still available on the system
    //
    bool found = false;
    QSerialPortInfo info;
    if(!m_portName.isEmpty())
    {
        QMap<QString,QSerialPortInfo>::const_iterator it = m_deviceList.constBegin();
        while(it != m_deviceList.constEnd() && !found)
        {
          found = it.key().contains(m_portName);
          if(found) info = it.value();
          ++it;
        }
    }
    if(found)
    {
        if(m_verbose)
            qDebug() << "found the ini stored port  " << m_portName;

       setDevice(info);
    }
    else
    {
        emit canSelect();
    }

    qDebug() << "done scanning for ports";
}

void AudiometerManager::selectDevice(const QString &label)
{
    if(m_deviceList.contains(label))
    {
      QSerialPortInfo info = m_deviceList.value(label);
      setProperty("portName",info.portName());
      setDevice(info);
      if(m_verbose)
         qDebug() << "device selected from label " <<  label;
    }
}

void AudiometerManager::setDevice(const QSerialPortInfo &info)
{
    if(m_portName.isEmpty() || info.isNull())
    {
        if(m_verbose)
            qDebug() << "ERROR: no port available";
    }
    if(m_verbose)
        qDebug() << "ready to connect to " << info.portName();

    clearData();

    // get the device data
    if(info.hasProductIdentifier())
      m_deviceData["Product ID"] = QString::number(info.productIdentifier());
    if(info.hasVendorIdentifier())
      m_deviceData["Vendor ID"] = QString::number(info.vendorIdentifier());
    if(!info.manufacturer().isEmpty())
      m_deviceData["Manufacturer"] = info.manufacturer();
    if(!info.portName().isEmpty())
      m_deviceData["Port Name"] = info.portName();
    if(!info.serialNumber().isEmpty())
      m_deviceData["Serial Number"] = info.serialNumber();
    if(!info.systemLocation().isEmpty())
      m_deviceData["System Location"] = info.systemLocation();
    if(!info.description().isEmpty())
      m_deviceData["Description"] = info.description();

    m_port.setPort(info);
    if(m_port.open(QSerialPort::ReadWrite))
    {
      qDebug() << "ok, can open port for readwrite";
      emit canConnect();
    }
    else
    {
      qDebug() << "error: failed to open serial port: " << m_port.errorString();
    }
    m_port.close();
}

void AudiometerManager::connectPort()
{
    if(m_state == State::CONNECTED)
    {
        this->disconnectPort();
    }

    if(m_port.open(QSerialPort::ReadWrite))
    {
      m_state = State::CONNECTED;
      m_port.setDataBits(QSerialPort::Data8);
      m_port.setParity(QSerialPort::NoParity);
      m_port.setStopBits(QSerialPort::OneStop);
      m_port.setBaudRate(QSerialPort::Baud9600);

      connect( &m_port, &QSerialPort::readyRead,
               this, &AudiometerManager::readData);

      connect(&m_port, &QSerialPort::errorOccurred,
              this,[this](QSerialPort::SerialPortError error){
                  qDebug() << "AN ERROR OCCURED: " << m_port.errorString();
              });
      connect(&m_port,&QSerialPort::dataTerminalReadyChanged,
              this,[this](bool set){
          qDebug() << "data terminal ready DTR changed to " << (set?"high":"low");
      });
      connect(&m_port,&QSerialPort::requestToSendChanged,
              this,[this](bool set){
          qDebug() << "request to send RTS changed to " << (set?"high":"low");
      });
      emit canMeasure();
    }
}

void AudiometerManager::disconnectPort()
{
    if(m_port.isOpen())
        m_port.close();

    m_state = State::DISCONNECTED;
}

bool AudiometerManager::isDefined(const QString &label) const
{
    bool defined = false;
    if(m_deviceList.contains(label))
    {
        QSerialPortInfo info = m_deviceList.value(label);
        defined = !info.isNull();
    }
    return defined;
}

void AudiometerManager::readData()
{
    QByteArray data = m_port.readAll();
    m_buffer += data;
    qDebug() << "read data got " << QString(data);

    if(HearingMeasurement::hasEndCode(m_buffer))
    {
        qDebug() << "finished reading messages, final size " << QString::number(m_buffer.size());
        qDebug() << QString(m_buffer);
        emit measured(QString(m_buffer));

        HearingMeasurement m;
        m.fromArray(m_buffer);
        qDebug() << m.getFlag();
        qDebug() << m.getExaminerID();
        qDebug() << m.getTestDateTime().toString();
        qDebug() << m.getCalibrationDate().toString();
        m.getLeftHearingTestLevels();
        qDebug() << m.getPatientID();
        m.getRightHearingTestLevels();
    }
    else
    {
        // accumulate data
    }
}

void AudiometerManager::measure()
{
    m_buffer.clear();
    m_buffer.reserve(1024);

    const char cmd[] = {0x05,'4',0x0d};
    QByteArray req = QByteArray::fromRawData(cmd,3);
    qDebug() << "writing byte array to port: " << QString(req);
    m_port.write(req);

    // send a signal when everything is read in
    // or an error occured with the port
    // so that the measure button can be enabled again

}

QJsonObject AudiometerManager::toJsonObject() const
{
    QMap<QString,QVariant>::const_iterator it = m_deviceData.constBegin();
    QJsonObject jsonObjDevice;
    while(it != m_deviceData.constEnd())
    {
        jsonObjDevice.insert(it.key(),QJsonValue::fromVariant(it.value()));
        ++it;
    }
    QList<HearingMeasurement>::const_iterator mit = m_measurementData.constBegin();
    QMap<QString,QJsonArray> jmap;
    while(mit != m_measurementData.constEnd())
    {
        QMap<QString,QVariant> c = mit->getCharacteristicValues();
        it = c.constBegin();
        while(it != c.constEnd())
        {
          if(!jmap.contains(it.key()))
          {
             jmap[it.key()] = QJsonArray();
          }
          jmap[it.key()].append(QJsonValue::fromVariant(it.value()));
          ++it;
        }
        ++mit;
    }
    QJsonObject jsonObjMeasurement;
    QMap<QString,QJsonArray>::const_iterator jit = jmap.constBegin();
    while(jit != jmap.constEnd())
    {
        jsonObjMeasurement.insert(jit.key(),jit.value());
        ++jit;
    }
    QJsonObject json;
    json.insert("device",QJsonValue(jsonObjDevice));
    json.insert("measurement",QJsonValue(jsonObjMeasurement));

    return json;
}
