#include "BluetoothLEManager.h"

#include <QBitArray>
#include <QDateTime>
#include <QSettings>
#include <QtBluetooth/QBluetoothHostInfo>
#include <QJsonArray>
#include <QJsonObject>

/**
 * Qt 5.14 adds a native Win32 port supporting Classic Bluetooth on Windows 7 or newer, and Bluetooth LE on Windows 8 or newer.
 * It must be enabled at build time by configuration option -native-win32-bluetooth. The UWP backend is used by default if this
 * option is not set and the Win32 target platform supports the required UWP APIs (minimal requirement is Windows 10 version 1507,
 * with slightly improved service discovery since Windows 10 version 1607).
 *
*/

BluetoothLEManager::BluetoothLEManager(QObject *parent) : QObject(parent),
    m_verbose(false),
    m_foundThermoService(false),
    m_foundInfoService(false)
{
}

void BluetoothLEManager::selectAdapter(const QString &address)
{
#ifdef __linux__
    if(!address.isEmpty())
    {
      m_client.reset(new QBluetoothLocalDevice(QBluetoothAddress(address)));
      if(m_verbose) qDebug() << "using adapter from settings " << address;
    }
    if(m_client.isNull())
    {
      QList<QBluetoothHostInfo> localAdapters = QBluetoothLocalDevice::allDevices();
      if(!localAdapters.empty())
      {
        m_client.reset(new QBluetoothLocalDevice(localAdapters.at(0).address()));
        if(m_verbose) qDebug() << "using adapter from list of known devices " << m_client->address().toString();
      }
    }

    if(m_client->isValid())
    {
      if(QBluetoothLocalDevice::HostPoweredOff == m_client->hostMode())
      {
        if(m_verbose) qDebug() << "local adapter is powered off";
        m_client->powerOn();
      }
      if(QBluetoothLocalDevice::HostDiscoverable != m_client->hostMode())
      {
        if(m_verbose) qDebug() << "setting local adapter host mode to host discoverable";
        m_client->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
      }
    }
    else
    {
        if(m_verbose) qDebug() << "error: could not find a valid adapter";
    }
 #endif
}

void BluetoothLEManager::loadSettings(const QSettings &settings)
{
    QString address = settings.value("client/address").toString();

    selectAdapter(address);

    // Get the thermometer MAC address.
    // If none exists perform device discovery process
    //
    address = settings.value("peripheral/address").toString();
    if(!address.isEmpty())
    {
      setProperty("peripheralMAC", address);
      if(m_verbose)
          qDebug() << "using peripheral MAC " << m_peripheralMAC << " from settings file";
    }
}

void BluetoothLEManager::saveSettings(QSettings *settings)
{
    if(!m_client.isNull() && m_client->isValid())
    {
      settings->setValue("client/name",m_client->name());
      settings->setValue("client/address",m_client->address().toString());
      settings->setValue("client/hostmode",m_client->hostMode());
      if(m_verbose)
          qDebug() << "wrote client to settings file";
    }
    if(!m_peripheral.isNull() && m_peripheral->isValid())
    {
      settings->setValue("peripheral/name",m_peripheral->name());
      settings->setValue("peripheral/address",m_peripheral->address().toString());
      if(m_verbose)
          qDebug() << "wrote peripheral to settings file";
    }
}

bool BluetoothLEManager::lowEnergyEnabled() const
{
  return (QBluetoothDeviceDiscoveryAgent::LowEnergyMethod &
          QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods());
}

bool BluetoothLEManager::localAdapterEnabled() const
{
    bool enabled = true;
#ifdef __linux__
    enabled = (!m_client.isNull() && m_client->isValid());
#endif
    return enabled;
}

void BluetoothLEManager::scanDevices()
{
    // NOTE: Due to API limitations it is only possible to find devices that have been paired
    // using Windows' settings on Win OS.
    // Create the agent to perform device discovery and populate the address list box
    // with candidate items.
    // If the address line edit field has not been filled with a stored peripheral address,
    // prompt the user to double click to select a device.
    //
    if(m_agent.isNull())
    {
      m_agent.reset(new QBluetoothDeviceDiscoveryAgent(this));

      connect(m_agent.data(), &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
           this, &BluetoothLEManager::deviceDiscovered);

      connect(m_agent.data(), QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error),
            this,[this](QBluetoothDeviceDiscoveryAgent::Error error)
        {
          QStringList s = QVariant::fromValue(error).toString().split(QRegExp("(?=[A-Z])"),QString::SkipEmptyParts);
          if(m_verbose)
              qDebug() << "device discovery error " << s.join(" ").toLower();
        }
      );

      connect(m_agent.data(), &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothLEManager::deviceDiscoveryComplete);

      connect(m_agent.data(), &QBluetoothDeviceDiscoveryAgent::canceled,
              this, &BluetoothLEManager::deviceDiscoveryComplete);
    }

    m_deviceList.clear();

    emit scanning();

    m_agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

bool BluetoothLEManager::isPairedTo(const QString &label) const
{
    bool paired = false;
    if(m_deviceList.contains(label) && !m_client.isNull())
    {
        QBluetoothDeviceInfo info = m_deviceList.value(label);
        QBluetoothLocalDevice::Pairing pairingStatus = m_client->pairingStatus(info.address());
        paired = (pairingStatus == QBluetoothLocalDevice::Paired || pairingStatus == QBluetoothLocalDevice::AuthorizedPaired);
    }
    return paired;
}

// add a device to the ui list
//
void BluetoothLEManager::deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    if(m_verbose)
        qDebug() << "Found device:" << info.name() << '(' << info.address().toString() << ')';

    // Add the device to the list
    //
    QString label = QString("%1 %2").arg(info.address().toString(),info.name());
    if(!m_deviceList.contains(label))
    {
        m_deviceList.insert(label,info);
        if(m_peripheralMAC == info.address().toString())
        {
            m_agent->stop();
        }
        emit discovered(label);
    }
}

// Enable discovered devices selection
//
void BluetoothLEManager::deviceDiscoveryComplete()
{
    QList<QBluetoothDeviceInfo> devices = m_agent->discoveredDevices();
    if(m_verbose)
        qDebug() << "Found " << QString::number(devices.count()) << " devices";

    // If no devices found, warn the user to check the client bluetooth adapter
    // and to pair any peripheral devices, then close the application
    //

    // If we recovered the peripheral MAC from .ini, verify that it is among the devices
    // discovered.  If it isn't, pop a warning dialog and prompt the user to either select
    // a device from the list or enter the MAC address manually.
    //
    bool found = false;
    QBluetoothDeviceInfo info;
    if(!m_peripheralMAC.isEmpty())
    {
      QMap<QString,QBluetoothDeviceInfo>::const_iterator it = m_deviceList.constBegin();
      while(it != m_deviceList.constEnd() && !found)
      {
        found = it.key().contains(m_peripheralMAC);
        if(found) info = it.value();
        ++it;
      }
    }
    if(found)
    {
        if(m_verbose)
            qDebug() << "found the peripheral with mac address " << m_peripheralMAC;
       // Initiate service discovery preparation
       //
       this->setDevice(info);
    }
    else
    {
        emit canSelect();
    }
}

void BluetoothLEManager::selectDevice(const QString &label)
{
    if(m_verbose)
        qDebug() << "device selected from list " <<  label;
    if(m_deviceList.contains(label))
    {
      QBluetoothDeviceInfo info = m_deviceList.value(label);
      setProperty("peripheralMAC",info.address().toString());
      setDevice(info);
    }
}

void BluetoothLEManager::serviceDiscovered(const QBluetoothUuid &uuid)
{
  if(m_verbose)
      qDebug() << "service discovered "<< uuid.toString();
  if(uuid == QBluetoothUuid(QBluetoothUuid::HealthThermometer))
  {
      if(m_verbose)
          qDebug() << "discovered the health thermometer service";
      m_foundThermoService = true;
  }
  else if(uuid == QBluetoothUuid(QBluetoothUuid::DeviceInformation))
  {
     if(m_verbose)
         qDebug() << "discovered the device information service";
     m_foundInfoService = true;
  }
}

void BluetoothLEManager::clearData()
{
    m_foundThermoService = false;
    m_foundInfoService = false;
    m_measurementData.clear();
    m_deviceData.clear();
    setProperty("temperature", "");
    setProperty("datetime", "");
}

void BluetoothLEManager::setDevice(const QBluetoothDeviceInfo &info)
{
    // sanity check
    //
    if(m_peripheralMAC.isEmpty() || !info.isValid())
    {
        if(m_verbose)
            qDebug() << "ERROR: no peripheral to search for services" <<
                     (m_peripheralMAC.isEmpty()?"empty peripheral mac " : "peripheral mac ok ") <<
                    (info.isValid()?"device info is valid ":"device info is invalid" ) <<
                    "terminating!";
       // TODO emit fatal error signal
    }

    if(m_verbose)
        qDebug() << "ready to connect to " << info.address().toString() << " and discover services";

    clearData();

    if(!m_controller.isNull())
    {
        m_controller->disconnectFromDevice();
    }

    m_deviceData["Device Name"] = info.name();
    m_deviceData["Device MAC"] = info.address().toString();

    m_peripheral.reset(new QBluetoothDeviceInfo(info));
    m_controller.reset(QLowEnergyController::createCentral(*m_peripheral));
    m_controller->setRemoteAddressType(QLowEnergyController::PublicAddress);

    connect(m_controller.data(), &QLowEnergyController::connected,
      this,[this](){
        if(m_verbose)
            qDebug() << "controller finding "<< m_controller->remoteName() << " services";
        m_controller->discoverServices();
        emit connected();
      }
    );

    connect(m_controller.data(), &QLowEnergyController::disconnected,
      this,[this](){
        if(m_verbose)
            qDebug() << "controller disconnected from " << m_controller->remoteName();
        emit canConnect();
      }
    );

    connect(m_controller.data(), &QLowEnergyController::serviceDiscovered,
      this, &BluetoothLEManager::serviceDiscovered);

    connect(m_controller.data(), QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error),
      this,[this](QLowEnergyController::Error error)
      {
        QStringList s = QVariant::fromValue(error).toString().split(QRegExp("(?=[A-Z])"),QString::SkipEmptyParts);
        if(m_verbose)
          qDebug() << "low energy controller error " << s.join(" ").toLower();
      }
    );

    connect(m_controller.data(), &QLowEnergyController::discoveryFinished,
      this, &BluetoothLEManager::serviceDiscoveryComplete);

    connect(m_controller.data(), &QLowEnergyController::stateChanged,
      this,[this](QLowEnergyController::ControllerState state){
        QStringList s = QVariant::fromValue(state).toString().split(QRegExp("(?=[A-Z])"),QString::SkipEmptyParts);
        if(m_verbose)
          qDebug() << "controller state changed to " << s.join(" ").toLower();
     });

    emit canConnect();
}

void BluetoothLEManager::connectPeripheral()
{
    if(!m_controller.isNull() && QLowEnergyController::UnconnectedState == m_controller->state())
    {
        if(m_verbose)
          qDebug() << "controller connect to device";
        m_controller->connectToDevice();
    }
}

void BluetoothLEManager::serviceDiscoveryComplete()
{
  if(m_verbose)
      qDebug() << "controller service discovery complete";

  m_thermo_service.reset();
  if(m_foundThermoService)
  {
    m_thermo_service.reset(
      m_controller->createServiceObject(QBluetoothUuid(QBluetoothUuid::HealthThermometer),
        m_controller.data()));
    if(!m_thermo_service.isNull())
    {
        connect(m_thermo_service.data(), &QLowEnergyService::stateChanged,
          this, &BluetoothLEManager::serviceStateChanged);
        connect(m_thermo_service.data(), &QLowEnergyService::characteristicChanged,
          this, &BluetoothLEManager::updateTemperatureData);
        connect(m_thermo_service.data(), &QLowEnergyService::descriptorWritten,
          this,[this](const QLowEnergyDescriptor &d, const QByteArray &a)
        {
          if(m_verbose)
              qDebug() << ((d.isValid() && a == QByteArray::fromHex("0100"))?"success descriptor write":"descriptor write error");
        }
      );
      m_thermo_service->discoverDetails();
    }
  }
  else
  {
      if(m_verbose)
          qDebug() << "health thermometer service not found";
  }

  m_info_service.reset();
  if(m_foundInfoService)
  {
    m_info_service.reset(
      m_controller->createServiceObject(QBluetoothUuid(QBluetoothUuid::DeviceInformation),
        m_controller.data()));
    if(!m_info_service.isNull())
    {
      connect(m_info_service.data(), &QLowEnergyService::stateChanged,
        this, &BluetoothLEManager::serviceStateChanged);
      connect(m_info_service.data(), &QLowEnergyService::characteristicRead,
        this, &BluetoothLEManager::updateInfoData);
      m_info_service->discoverDetails();
    }
  }
  else
  {
      if(m_verbose)
          qDebug() << "device information service not found";
  }
}

void BluetoothLEManager::serviceStateChanged(QLowEnergyService::ServiceState state)
{
    auto service = qobject_cast<QLowEnergyService *>(sender());

    QString name = service->serviceName();
    QStringList s = QVariant::fromValue(state).toString().split(QRegExp("(?=[A-Z])"),QString::SkipEmptyParts);
    if(m_verbose)
        qDebug() << name << " service state changed to " << s.join(" ").toLower();;

    if(QLowEnergyService::ServiceDiscovered != state)
        return;

    if("Health Thermometer" == name )
    {
      const QLowEnergyCharacteristic lecTM = service->characteristic(QBluetoothUuid(QBluetoothUuid::TemperatureMeasurement));
      if (!lecTM.isValid())
      {
        if(m_verbose)
            qDebug() << lecTM.name() << " is invalid.";
        return;
      }

      QLowEnergyDescriptor ledN = lecTM.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
      if(ledN.isValid())
      {
          if(m_verbose)
              qDebug() << ledN.name() << " found ... writing";
          service->writeDescriptor(ledN, QByteArray::fromHex("0100"));
      }
      else
      {
          if(m_verbose)
              qDebug() << "failed to write descriptor " << ledN.name() << " for service " << name;
      }
    }
    else if("Device Information" == name)
    {
        const QLowEnergyCharacteristic lecFR = service->characteristic(QBluetoothUuid(QBluetoothUuid::FirmwareRevisionString));
        if (!lecFR.isValid())
        {
            if(m_verbose)
                qDebug() << lecFR.name() << " is invalid.";
            return;
        }
        else
        {
          service->readCharacteristic(lecFR);
        }
        const QLowEnergyCharacteristic lecSW = service->characteristic(QBluetoothUuid(QBluetoothUuid::SoftwareRevisionString));
        if (!lecSW.isValid())
        {
            if(m_verbose)
                qDebug() << lecSW.name() << " is invalid.";
            return;
        }
        else
        {
          service->readCharacteristic(lecSW);
        }
    }
}

void BluetoothLEManager::disconnectPeripheral()
{
     if(!m_measurementData.isEmpty() &&
         m_deviceData.contains("Firmware Revision") &&
         m_deviceData.contains("Software Revision")    )
     {
         if(m_verbose)
             qDebug() << "data complete, ready to disconnect, emitting canWrite signal";
         emit canWrite();

         if(QLowEnergyController::DiscoveredState == m_controller->state())
           m_controller->disconnectFromDevice();
     }
}

void BluetoothLEManager::updateInfoData(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    if(c.uuid() == QBluetoothUuid(QBluetoothUuid::FirmwareRevisionString))
    {
      if(m_verbose)
          qDebug() << "device info firmware revision update";
      m_deviceData["Firmware Revision"] = (c.isValid() ? QVariant(value) : QVariant());
    }
    else if(c.uuid() == QBluetoothUuid(QBluetoothUuid::SoftwareRevisionString))
    {
      if(m_verbose)
          qDebug() << "device info software revision update";
      m_deviceData["Software Revision"] = (c.isValid() ? QVariant(value) : QVariant());
    }

    disconnectPeripheral();
}

void BluetoothLEManager::updateTemperatureData(const QLowEnergyCharacteristic &c, const QByteArray &arr)
{
  if(m_verbose)
      qDebug() << "temperature values update";
  if(c.uuid() != QBluetoothUuid(QBluetoothUuid::TemperatureMeasurement))
  {
      if(m_verbose)
          qDebug() << "update temperature error: empty data";
      return;
  }

  TemperatureMeasurement m;
  m.fromArray(arr);
  if(m.isValid())
  {
    m_measurementData.clear();
    m_measurementData.push_back(m);
    emit measured(m.toString());
    emit canWrite();
  }

   disconnectPeripheral();
}

QJsonObject BluetoothLEManager::toJsonObject() const
{
    QMap<QString,QVariant>::const_iterator it = m_deviceData.constBegin();
    QJsonObject jsonObjDevice;
    while(it != m_deviceData.constEnd())
    {
        jsonObjDevice.insert(it.key(),QJsonValue::fromVariant(it.value()));
        ++it;
    }
    QList<TemperatureMeasurement>::const_iterator mit = m_measurementData.constBegin();
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
