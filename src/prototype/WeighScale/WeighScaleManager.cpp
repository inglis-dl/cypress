#include "WeighScaleManager.h"

#include <QDateTime>
#include <QDebug>
#include <QSerialPortInfo>
#include <QSettings>
#include <QtMath>

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
    QString portName = settings.value("client/port").toString();
    if(!portName.isEmpty())
    {
      setProperty("portName", portName);
      if(m_verbose)
          qDebug() << "using serial port " << m_portName << " from settings file";
    }
}

void WeighScaleManager::saveSettings(QSettings *settings)
{
    if(!m_portName.isNull())
    {
      settings->setValue("client/port",m_portName);
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
    m_portInfo = QSerialPortInfo();
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
            qDebug() << "found the port  " << m_portName;

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
      m_portInfo = QSerialPortInfo();
      QSerialPortInfo info = m_deviceList.value(label);
      setProperty("portName",info.portName());
      setDevice(info);
      if(m_verbose)
         qDebug() << "device selected from label " <<  label;
    }
}

void WeighScaleManager::setDevice(const QSerialPortInfo &info)
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

    //QSerialPort port(info);
    m_port.setPort(info);
    m_port.setDataBits(QSerialPort::Data8);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setBaudRate(QSerialPort::Baud9600);
    if(m_port.open(QSerialPort::ReadWrite))
    {
      m_portInfo = info;
      emit canConnect();
    }
    else
    {
        qDebug() << "error: failed to open serial port: " << m_port.errorString();
    }
    m_port.close();
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

void WeighScaleManager::zero()
{
    if(m_port.open(QSerialPort::ReadWrite))
    {
        m_port.write(QByteArray("z"));
        while(m_port.waitForBytesWritten(1000))
        {
        }
        m_port.write(QByteArray("p"));
        if(m_port.waitForReadyRead(1000))
        {
            QByteArray arr = m_port.readAll();
            while (m_port.waitForReadyRead(10))
                arr += m_port.readAll();
            if(!arr.isEmpty())
            {
              arr.chop(2);
              arr = arr.trimmed();
              QList<QByteArray> parts = arr.split(' ');
              if(3<=parts.size())
              {
                 double value = qFabs(QVariant(parts.first()).toDouble());
                 if(0.0==value)
                 {
                     qDebug() << "ok, scale confirmed zero: " << QString(parts.first());
                     emit canMeasure();
                 }
              }
            }
        }
        m_port.close();
    }
} 

void WeighScaleManager::measure()
{
    if(m_port.open(QSerialPort::ReadWrite))
    {
        m_port.write(QByteArray("p"));
        if(m_port.waitForReadyRead(1000))
        {
            QByteArray arr = m_port.readAll();
            while (m_port.waitForReadyRead(10))
                arr += m_port.readAll();
            if(!arr.isEmpty())
            {
              arr.chop(2);
              arr = arr.trimmed();
              QList<QByteArray> parts = arr.split(' ');
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

                  emit canWrite();
              }
            }
        }
        m_port.close();
    }
}
