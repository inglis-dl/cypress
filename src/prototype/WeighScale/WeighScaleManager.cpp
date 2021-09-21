#include "WeighScaleManager.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QSerialPortInfo>
#include <QSettings>
#include <QtMath>

WeighScaleManager::WeighScaleManager(QObject *parent) : QObject(parent),
    m_verbose(false),
    m_numberOfMeasurements(1)
{
}

bool WeighScaleManager::localPortsEnabled() const
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

void WeighScaleManager::saveSettings(QSettings *settings) const
{
    if(!m_portName.isEmpty())
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
}

void WeighScaleManager::scanDevices()
{
    m_deviceList.clear();
    emit scanning();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
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

       setDevice(info);
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

    m_port.setPort(info);
    m_port.setDataBits(QSerialPort::Data8);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setBaudRate(QSerialPort::Baud9600);
    if(m_port.open(QSerialPort::ReadWrite))
    {
      // try to read the id of the scale
        m_port.write(QByteArray("i"));
        QByteArray arr;
        if(m_port.waitForReadyRead(1000))
        {
            arr = m_port.readAll();
            while(m_port.waitForReadyRead(10))
                arr += m_port.readAll();

        }
        if(!arr.isEmpty())
        {
            arr = arr.simplified();
            m_deviceData["Scale Software ID"] = arr;
            qDebug() << "scale ID found " << arr;
            emit canConnect();
        }
        else
            qDebug() << "no scale id, array returned empty, port error " << m_port.errorString();
    }
    else
    {
      qDebug() << "error: failed to open serial port: " << m_port.errorString();
    }
    m_port.close();
}

bool WeighScaleManager::isDefined(const QString &label) const
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
            while(m_port.waitForReadyRead(10))
                arr += m_port.readAll();

            Measurement m;
            m.fromArray(arr);
            if(m.isZero())
              emit canMeasure();

            qDebug() << m;
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

            Measurement m;
            m.fromArray(arr);
            if(m.isValid())
            {
                m_measurementData.push_back(m);
                emit measured(m.toString());
                if(m_numberOfMeasurements <= m_measurementData.size())
                    emit canWrite();
            }
            qDebug() << m;
        }
        m_port.close();
    }
}

QJsonObject WeighScaleManager::toJsonObject() const
{
    QMap<QString,QVariant>::const_iterator it = m_deviceData.constBegin();
    QJsonObject jsonD;
    while(it != m_deviceData.constEnd())
    {
        jsonD.insert(it.key(),QJsonValue::fromVariant(it.value()));
        ++it;
    }
    QList<Measurement>::const_iterator mit = m_measurementData.constBegin();
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
    QJsonObject jsonM;
    QMap<QString,QJsonArray>::const_iterator jit = jmap.constBegin();
    while(jit != jmap.constEnd())
    {
        jsonM.insert(jit.key(),jit.value());
        ++jit;
    }
    QJsonObject json;
    json.insert("device",QJsonValue(jsonD));
    json.insert("measurement",QJsonValue(jsonM));

    return json;
}
