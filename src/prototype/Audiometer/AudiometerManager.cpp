#include "AudiometerManager.h"

#include "../../data/AudiometerTest.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QSerialPortInfo>
#include <QSettings>
#include <QtMath>

AudiometerManager::AudiometerManager(QObject *parent) : QObject(parent),
    m_verbose(false)
{
}

void AudiometerManager::buildModel(QStandardItemModel* model)
{
    HearingMeasurement m;

    QVector<QString> v_side({"left","right"});

    for(auto&& side : v_side)
    {
      int i_freq = 0;
      int col = "left" == side ? 0 : 1;
      for(int row=0;row<8;row++)
      {
        m = m_test.getMeasurement(side,i_freq);
        if(!m.isValid())
           m.fromCode(side,i_freq,"AA");
        QStandardItem* item = model->item(row,col);
        item->setData(m.toString(), Qt::DisplayRole);
        i_freq++;
      }
    }
}

bool AudiometerManager::devicesAvailable() const
{
    qDebug() << "checking available ports";
    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    bool ok = !list.empty();
    qDebug() << "got list result and returning";
    return ok;
}

void AudiometerManager::loadSettings(const QSettings &settings)
{
    QString name = settings.value("client/port").toString();
    if(!name.isEmpty())
    {
      setProperty("deviceName", name);
      if(m_verbose)
          qDebug() << "using serial port " << m_deviceName << " from settings file";
    }
}

void AudiometerManager::saveSettings(QSettings *settings) const
{
    if(!m_deviceName.isEmpty())
    {
      settings->setValue("client/port",m_deviceName);
      if(m_verbose)
          qDebug() << "wrote serial port to settings file";
    }
}

void AudiometerManager::clearData()
{
    m_deviceData.reset();
    m_test.reset();

    emit dataChanged();
}

void AudiometerManager::scanDevices()
{
    m_deviceList.clear();
    emit scanningDevices();
    qDebug() << "start scanning for devices ....";

    for(auto&& info : QSerialPortInfo::availablePorts())
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
        QString label = info.portName();
        if(!m_deviceList.contains(label))
        {
            m_deviceList.insert(label,info);
            emit deviceDiscovered(label);
            qDebug() << "found device " << label;
        }
    }
    if(m_verbose)
        qDebug() << "Found " << QString::number(m_deviceList.count()) << " ports";

    // if we have a port from the ini file, check if it is still available on the system
    //
    bool found = false;
    QSerialPortInfo info;
    if(!m_deviceName.isEmpty())
    {
        QMap<QString,QSerialPortInfo>::const_iterator it = m_deviceList.constBegin();
        while(it != m_deviceList.constEnd() && !found)
        {
          found = it.key() == m_deviceName;
          if(found) info = it.value();
          ++it;
        }
    }
    if(found)
    {
      if(m_verbose)
        qDebug() << "found the ini stored port  " << m_deviceName;

      setDevice(info);
    }
    else
    {
      // select a serial port from the list of scanned ports
      //
      emit canSelectDevice();
    }

    qDebug() << "done scanning for ports";
}

void AudiometerManager::selectDevice(const QString &label)
{
    if(m_deviceList.contains(label))
    {
      QSerialPortInfo info = m_deviceList.value(label);
      setProperty("deviceName",info.portName());
      setDevice(info);
      if(m_verbose)
         qDebug() << "device selected from label " <<  label;
    }
}

void AudiometerManager::setDevice(const QSerialPortInfo &info)
{
    if(m_deviceName.isEmpty() || info.isNull())
    {
        if(m_verbose)
            qDebug() << "ERROR: no port available";
    }
    if(m_verbose)
        qDebug() << "ready to connect to " << info.portName();

    clearData();

    // get the device data
    if(info.hasProductIdentifier())
      m_deviceData.setCharacteristic("port product ID", info.productIdentifier());
    if(info.hasVendorIdentifier())
      m_deviceData.setCharacteristic("port vendor ID", info.vendorIdentifier());
    if(!info.manufacturer().isEmpty())
      m_deviceData.setCharacteristic("port manufacturer", info.manufacturer());
    if(!info.portName().isEmpty())
      m_deviceData.setCharacteristic("port name", info.portName());
    if(!info.serialNumber().isEmpty())
      m_deviceData.setCharacteristic("port serial number", info.serialNumber());
    if(!info.systemLocation().isEmpty())
      m_deviceData.setCharacteristic("port system location", info.systemLocation());
    if(!info.description().isEmpty())
      m_deviceData.setCharacteristic("port description", info.description());

    m_port.setPort(info);
    if(m_port.open(QSerialPort::ReadWrite))
    {
      qDebug() << "ok, can open port for readwrite";

      // signal the GUI that the port is connectable so that
      // the connect button can be clicked
      //
      emit canConnectDevice();
    }
    else
    {
      qDebug() << "error: failed to open serial port: " << m_port.errorString();
    }
    m_port.close();
}

void AudiometerManager::connectDevice()
{
    if(m_port.open(QSerialPort::ReadWrite))
    {
      m_port.setDataBits(QSerialPort::Data8);
      m_port.setParity(QSerialPort::NoParity);
      m_port.setStopBits(QSerialPort::OneStop);
      m_port.setBaudRate(QSerialPort::Baud9600);

      connect(&m_port, &QSerialPort::readyRead,
               this, &AudiometerManager::readDevice);

      connect(&m_port, &QSerialPort::errorOccurred,
              this,[this](QSerialPort::SerialPortError error){
                  Q_UNUSED(error)
                  qDebug() << "AN ERROR OCCURED: " << m_port.errorString();
              });

      connect(&m_port, &QSerialPort::dataTerminalReadyChanged,
              this,[](bool set){
          qDebug() << "data terminal ready DTR changed to " << (set?"high":"low");
      });

      connect(&m_port, &QSerialPort::requestToSendChanged,
              this,[](bool set){
          qDebug() << "request to send RTS changed to " << (set?"high":"low");
      });

      // signal the GUI that the measure button can be clicked
      //
      emit canMeasure();
    }
}

void AudiometerManager::disconnectDevice()
{
    if(m_port.isOpen())
        m_port.close();

    clearData();
    emit canConnectDevice();
}

bool AudiometerManager::hasEndCode(const QByteArray &arr)
{
    if( arr.size() < 6 ) return false;
    // try and interpret the last 6 bytes
    int size = arr.size();
    return (
       0x0d == arr.at(size-1) &&
       0x17 == arr.at(size-4) &&
        'p' == arr.at(size-5) &&
        '~' == arr.at(size-6));
}

void AudiometerManager::readDevice()
{
    QByteArray data = m_port.readAll();
    m_buffer += data;
    qDebug() << "read data got " << QString(data);

    if(hasEndCode(m_buffer))
    {
        qDebug() << "finished reading messages, final size " << QString::number(m_buffer.size());
        qDebug() << QString(m_buffer);
        qDebug() << "manager ready to initialize test from buffer";

        m_test.fromArray(m_buffer);

        qDebug() << "manager ready to dump test to string";

        qDebug() << m_test.toString();
        if(m_test.isValid())
        {
            // emit the can write signal
            emit canWrite();
        }

        emit dataChanged();
        // TODO emit error
    }
}

void AudiometerManager::writeDevice()
{
    // prepare to receive data
    //
    m_buffer.clear();
    m_buffer.reserve(1024);

    // send the request to the device and then read the data
    // in readData()
    //
    const char cmd[] = {0x05,'4',0x0d};
    QByteArray req = QByteArray::fromRawData(cmd,3);
    m_port.write(req);
}

QJsonObject AudiometerManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    json.insert("device",m_deviceData.toJsonObject());
    return json;
}
