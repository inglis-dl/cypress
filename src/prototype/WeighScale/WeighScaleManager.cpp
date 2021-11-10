#include "WeighScaleManager.h"

#include "../../data/WeighScaleTest.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QSerialPortInfo>
#include <QSettings>
#include <QtMath>

WeighScaleManager::WeighScaleManager(QObject *parent) : QObject(parent),
    m_verbose(false),
    m_mode("default")
{
  m_test.setMaximumNumberOfMeasurements(2);
}

void WeighScaleManager::buildModel(QStandardItemModel* model)
{
    for(int row = 0; row < m_test.getMaximumNumberOfMeasurements(); row++)
    {
        QString s = "NA";
        WeightMeasurement m = m_test.getMeasurement(row);
        if(m.isValid())
           s = m.toString();
        QStandardItem* item = model->item(row,0);
        item->setData(s, Qt::DisplayRole);
    }
}

bool WeighScaleManager::devicesAvailable() const
{
    if("simulate" == m_mode)
    {
        return true;
    }
    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    bool ok = !list.empty();
    return ok;
}

void WeighScaleManager::loadSettings(const QSettings &settings)
{
    QString name = settings.value("client/port").toString();
    if(!name.isEmpty())
    {
      setProperty("deviceName", name);
      if(m_verbose)
          qDebug() << "using serial port " << m_deviceName << " from settings file";
    }
}

void WeighScaleManager::saveSettings(QSettings *settings) const
{
    if(!m_deviceName.isEmpty())
    {
      settings->setValue("client/port",m_deviceName);
      if(m_verbose)
          qDebug() << "wrote serial port to settings file";
    }
}

void WeighScaleManager::clearData()
{
    m_deviceData.reset();
    m_test.reset();

    emit dataChanged();
}

void WeighScaleManager::scanDevices()
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
}

void WeighScaleManager::selectDevice(const QString &label)
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

void WeighScaleManager::setDevice(const QSerialPortInfo &info)
{
    if("simulate" == m_mode)
    {
       clearData();
       // get the device data
       m_deviceData.setCharacteristic("port product ID", "simulated produce ID");
       m_deviceData.setCharacteristic("port vendor ID", "simulated vendor ID");
       m_deviceData.setCharacteristic("port manufacturer", "simulated manufacturer");
       m_deviceData.setCharacteristic("port name", (m_deviceName.isEmpty() ? "simulated_device" : m_deviceName));
       m_deviceData.setCharacteristic("port serial number", "simulated serial number");
       m_deviceData.setCharacteristic("port system location", "simulated system location");
       m_deviceData.setCharacteristic("port description", "simulated description");
       emit canConnectDevice();
       return;
    }

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
      // signal the GUI that the port is connectable so that
      // the connect button can be clicked
      //
      emit canConnectDevice();
    }
    m_port.close();
}

void WeighScaleManager::connectDevice()
{
    if("simulate" == m_mode)
    {
        m_request = QByteArray("i");
        writeDevice();
        return;
    }

    if(m_port.open(QSerialPort::ReadWrite))
    {
      m_port.setDataBits(QSerialPort::Data8);
      m_port.setParity(QSerialPort::NoParity);
      m_port.setStopBits(QSerialPort::OneStop);
      m_port.setBaudRate(QSerialPort::Baud9600);

      connect(&m_port, &QSerialPort::readyRead,
               this, &WeighScaleManager::readDevice);

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

      // try and read the scale ID, if we can do that then emit the
      // canMeasure signal
      // the canMeasure signal is emitted from readDevice slot on successful read
      //
      m_request = QByteArray("i");
      writeDevice();
    }
}

void WeighScaleManager::writeDevice()
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

void WeighScaleManager::disconnectDevice()
{
    clearData();
    emit canConnectDevice();
    if("simulate" == m_mode)
    {
       return;
    }

    if(m_port.isOpen())
        m_port.close();
}

bool WeighScaleManager::isDefined(const QString &label) const
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

void WeighScaleManager::zeroDevice()
{
    m_request = QByteArray("z");
    writeDevice();
}

void WeighScaleManager::measure()
{
    m_request = QByteArray("p");
    writeDevice();
}

void WeighScaleManager::readDevice()
{
    if("simulate" == m_mode)
    {
        QString simdata;
        if("i" == QString(m_request))
         {
           simdata = "12345";
         }
         else if("z" == QString(m_request))
         {
           simdata = "0.0 C body";
         }
         else if("p" == QString(m_request))
         {
            simdata = "36.1 C body";
         }
         m_buffer = QByteArray(simdata.toUtf8());
    }
    else
    {
      QByteArray data = m_port.readAll();
      m_buffer += data;
    }
    if(m_verbose)
      qDebug() << "read device received buffer " << QString(m_buffer);

    if(!m_buffer.isEmpty())
    {
      if("i" == QString(m_request))
      {
        m_deviceData.setCharacteristic("software ID", QString(m_buffer.simplified()));
        emit canMeasure();
      }
      else if("p" == QString(m_request))
      {
         m_test.fromArray(m_buffer);
         if(m_test.isValid())
         {
             // emit the can write signal
             emit canWrite();
         }
      }
      else if("z" == QString(m_request))
      {
          WeightMeasurement m;
          m.fromArray(m_buffer);
          if(m.isZero())
            emit canMeasure();
      }

      emit dataChanged();
    }
}

QJsonObject WeighScaleManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    json.insert("device",m_deviceData.toJsonObject());
    return json;
}
