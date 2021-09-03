#include "WeighScaleManager.h"

#include <QDateTime>
#include <QDebug>
#include <QSerialPortInfo>
#include <QSettings>

WeighScaleManager::WeighScaleManager(QObject *parent) : QObject(parent),
    m_verbose(false)
{
}

bool WeighScaleManager::localPortsEnabled()
{
    return !QSerialPortInfo::availablePorts().empty();
}

void WeighScaleManager::loadSettings(const QSettings &settings)
{
    QString port = settings.value("client/port").toString();

    if(!port.isEmpty())
    {
      setProperty("serialPort", port);
      if(m_verbose)
          qDebug() << "using serial port " << m_serialPort << " from settings file";
    }
}

void WeighScaleManager::saveSettings(QSettings *settings)
{
    if(!m_port.isNull())
    {
      settings->setValue("client/port",m_port->portName());
      if(m_verbose)
          qDebug() << "wrote serial port to settings file";
    }
}

void WeighScaleManager::clearData()
{
    m_measurementData.clear();
    m_deviceData.clear();
    setProperty("weight", "");
    setProperty("datetime", "");
}

void WeighScaleManager::scanDevices()
{
    m_deviceList.clear();
    emit scanning();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
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

        QString label = QString("%1 %2").arg(info.portName(),info.description());
        if(!m_deviceList.contains(label))
        {
            m_deviceList.insert(label,info);
            emit discovered(label);
        }
    }
    if(m_verbose)
        qDebug() << "Found " << QString::number(m_deviceList.count()) << " ports";
    bool found = false;
    QSerialPortInfo info;
    if(!m_serialPort.isEmpty())
    {
        QMap<QString,QSerialPortInfo>::const_iterator it = m_deviceList.constBegin();
        while(it != m_deviceList.constEnd() && !found)
        {
          found = it.key().contains(m_serialPort);
          if(found) info = it.value();
          ++it;
        }
    }
    if(found)
    {
        if(m_verbose)
            qDebug() << "found the port with serial " << m_serialPort;
       // Initiate service discovery preparation
       //
       this->setDevice(info);
    }
    else
    {
        emit canSelect();
    }
}

void WeighScaleManager::selectDevice(const QString &label)
{
    if(m_deviceList.contains(label))
    {
      QSerialPortInfo info = m_deviceList.value(label);
      setProperty("serialPort",info.portName());
      setDevice(info);
      if(m_verbose)
         qDebug() << "device selected from label " <<  label;
    }
}

void WeighScaleManager::setDevice(const QSerialPortInfo &info)
{
    if(m_serialPort.isEmpty() || info.isNull())
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

    m_port.reset(new QSerialPort(info));
    if(m_port->open(QIODevice::ReadWrite))
    {
        qDebug() << "port " << info.portName() << " can read write";
        connect(m_port.data(),&QSerialPort::errorOccurred,
                this,[this](QSerialPort::SerialPortError error){
            if(m_verbose)
            {
                QStringList s = QVariant::fromValue(error).toString().split(QRegExp("(?=[A-Z])"),QString::SkipEmptyParts);
                qDebug() << "serial port reported error " << s.join(" ").toLower();
            }
        }
        );
        emit canConnect();
    }
    else
    {
        qDebug() << "port " << info.portName() << " can not read from";
    }
    if(m_port->isOpen())
    {
       m_port->close();
    }
}

bool WeighScaleManager::isDefined(const QString &label)
{
    bool defined = false;
    if(m_deviceList.contains(label))
    {
        QSerialPortInfo info = m_deviceList.value(label);
        defined = !info.isNull();
    }
    return defined;
}

void WeighScaleManager::connectToPort()
{
    if(m_port.isNull())
    {
        qDebug() << "ERROR: the port object is not allocated";
       return;
    }
    m_port->open(QIODevice::ReadWrite);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setBaudRate(QSerialPort::Baud9600);

    emit connected();
}

void WeighScaleManager::zero()
{
    if(m_port->isOpen())
    {
      QByteArray z("z");
      m_port->write(z);
      if(QSerialPort::NoError==m_port->error())
      {
          qDebug() << "success zeroing";
      }
      else
      {
          qDebug() << "error occured zeroing " << m_port->errorString();
      }
      emit canMeasure();
    }
}

void WeighScaleManager::measure()
{
    if(m_port->isOpen())
    {
      QByteArray p("p");
      m_port->write(p);
      if(QSerialPort::NoError==m_port->error())
      {
          qDebug() << "success reading";
          QByteArray output = m_port->readAll();
          QString s(output);
          qDebug() << "output: " << s ;
          // parse the output
          // read from the end of the string back
          // format is
          // ...<SP>xxxxxxxx<SP>uu<SP>mmmmm<SP><CR><LF>
          // xxxxxxx is the weight with decimal point and "_" sign,
          // if neg uu is the unit (lb or kg)
          // mmmmm is the mode (gross or net)
          // strip off the last two chars
          // split on <SP>
          // loop and trim results
          output.chop(2);
          output = output.trimmed();
          QList<QByteArray> parts = output.split(' ');
          if(3<=parts.size())
          {
              QString weightStr = QString(parts[0]);
              QString unitStr = QString(parts[1]);
              QString modeStr = QString(parts[2]);
              QDateTime dt = QDateTime::currentDateTime();
              QString d = dt.date().toString("yyyy-MM-dd");
              QString t = dt.time().toString("hh:mm:ss");

              qDebug() << "parsed output " << weightStr
                       << "(" << unitStr << ")"
                       << " " << modeStr << ", "
                       << d << " " << t;

          }
      }
      else
      {
          qDebug() << "error occured reading " << m_port->errorString();
      }
      emit canWrite();
    }
}
