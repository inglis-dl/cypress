#include "SerialPortManager.h"

#include <QDebug>
#include <QJsonObject>
#include <QSettings>

SerialPortManager::SerialPortManager(QObject *parent) : ManagerBase(parent)
{
}

bool SerialPortManager::isDefined(const QString &label) const
{
    if("simulate" == m_mode)
    {
       return true;
    }
    bool defined = false;
    if(m_deviceList.contains(label))
    {
        QSerialPortInfo info = m_deviceList.value(label);
        defined = !info.isNull();
    }
    return defined;
}

bool SerialPortManager::devicesAvailable() const
{
    if("simulate" == m_mode)
    {
        return true;
    }
    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    return !list.empty();
}

void SerialPortManager::loadSettings(const QSettings &settings)
{
    QString name = settings.value("client/port").toString();
    if(!name.isEmpty())
    {
      setProperty("deviceName", name);
      if(m_verbose)
          qDebug() << "using serial port " << m_deviceName << " from settings file";
    }
}

void SerialPortManager::saveSettings(QSettings *settings) const
{
    if(!m_deviceName.isEmpty())
    {
      settings->setValue("client/port",m_deviceName);
      if(m_verbose)
          qDebug() << "wrote serial port to settings file";
    }
}

void SerialPortManager::scanDevices()
{
    m_deviceList.clear();
    emit scanningDevices();
    if(m_verbose)
      qDebug() << "start scanning for devices ....";

    if("simulate" == m_mode)
    {
      QSerialPortInfo info;
      QString label = m_deviceName.isEmpty() ? "simulated_device" : m_deviceName;
      m_deviceList.insert(label,info);
      emit deviceDiscovered(label);
      if(m_verbose)
        qDebug() << "found device " << label;
      setDevice(info);
      return;
    }

    for(auto&& info : QSerialPortInfo::availablePorts())
    {
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
        }
    }
    if(m_verbose)
        qDebug() << "Found " << QString::number(m_deviceList.count()) << " ports";

    // if we have a port from the ini file, check if it is still available on the system
    //
    bool found = false;
    QSerialPortInfo info;
    QString label;
    if(!m_deviceName.isEmpty())
    {
        QMap<QString,QSerialPortInfo>::const_iterator it = m_deviceList.constBegin();
        while(it != m_deviceList.constEnd() && !found)
        {
          label = it.key();
          found = label == m_deviceName;
          if(found) info = it.value();
          ++it;
        }
    }
    if(found)
    {
      if(m_verbose)
        qDebug() << "found the ini stored port  " << m_deviceName;

      emit deviceSelected(label);
      setDevice(info);
    }
    else
    {
      // select a serial port from the list of scanned ports
      //
      emit canSelectDevice();
    }
}

void SerialPortManager::selectDevice(const QString &label)
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

void SerialPortManager::setDevice(const QSerialPortInfo &info)
{
    if(m_port.portName() == info.portName())
    {
        return;
    }

    m_deviceData.reset();

    if("simulate" == m_mode)
    {
       // get the device data
       m_deviceData.setCharacteristic("port product ID", "simulated");
       m_deviceData.setCharacteristic("port vendor ID", "simulated");
       m_deviceData.setCharacteristic("port manufacturer", "simulated");
       m_deviceData.setCharacteristic("port name", (m_deviceName.isEmpty() ? "simulated" : m_deviceName));
       m_deviceData.setCharacteristic("port serial number", "simulated");
       m_deviceData.setCharacteristic("port system location", "simulated");
       m_deviceData.setCharacteristic("port description", "simulated");
       emit canConnectDevice();
       return;
    }

    if(m_deviceName.isEmpty() || info.isNull())
    {
        if(m_verbose)
            qDebug() << "ERROR: no port available";
        return;
    }
    if(m_verbose)
        qDebug() << "ready to connect to " << info.portName();

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
      // signal the GUI that the port is connectable so that
      // the connect button can be clicked
      //
      emit canConnectDevice();
    }
    m_port.close();
}

void SerialPortManager::connectDevice()
{
    if("simulate" == m_mode)
    {
        emit canMeasure();
        return;
    }

    if(m_port.open(QSerialPort::ReadWrite))
    {
      m_port.setDataBits(QSerialPort::Data8);
      m_port.setParity(QSerialPort::NoParity);
      m_port.setStopBits(QSerialPort::OneStop);
      m_port.setBaudRate(QSerialPort::Baud9600);

      connect(&m_port, &QSerialPort::readyRead,
               this, &SerialPortManager::readDevice);

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

void SerialPortManager::disconnectDevice()
{
    emit canConnectDevice();
    if("simulate" == m_mode)
    {
       return;
    }

    if(m_port.isOpen())
        m_port.close();
}

QJsonObject SerialPortManager::toJsonObject() const
{
    QJsonObject json;
    json.insert("device",m_deviceData.toJsonObject());
    return json;
}

void SerialPortManager::writeDevice()
{
    // prepare to receive data
    //
    m_buffer.clear();

    if("simulate" == m_mode)
    {
        if(m_verbose)
          qDebug() << "in simulate mode writeDevice with request " << QString(m_request);
        readDevice();
        return;
    }

    m_port.write(m_request);
}
