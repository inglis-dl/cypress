#include "BluetoothLEManager.h"

#include <QBitArray>
#include <QDateTime>
#include <QSettings>
#include <QtBluetooth/QBluetoothHostInfo>

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

bool BluetoothLEManager::lowEnergyEnabled()
{
  return (QBluetoothDeviceDiscoveryAgent::LowEnergyMethod &
          QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods());
}

bool BluetoothLEManager::localAdapterEnabled()
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

bool BluetoothLEManager::isPairedTo(const QString &label)
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

void BluetoothLEManager::updateTemperatureData(const QLowEnergyCharacteristic &c, const QByteArray &a)
{
  if(m_verbose)
      qDebug() << "temperature values update";
  if(c.uuid() != QBluetoothUuid(QBluetoothUuid::TemperatureMeasurement))
  {
      if(m_verbose)
          qDebug() << "update temperature error: empty data";
      return;
  }

  /**
   * The Temperature Measurement Value field may contain special float value NaN
(0x007FFFFF) defined in IEEE 11073-20601 [4] to report an invalid result from a
computation step or missing data due to the hardware’s inability to provide a valid
measurement
   *
   * */
  /**
   * flags field: 8bit
   * temperature field: sint16
   * datetime field:
   * year uint16
   * month uint8
   * day uint8
   * hours (past midnight) uint8
   * minutes (since start of hour) uint8
   * seconds (since start of minute) uint8
   * temperature type: 8bit
   *
   * example: 07 d3 03 00 ff e5 07 07 08 0f 22 00 01
   * flags: 07, hex to binary = 00000111, bit 0 = 1 => Fahrenheit, bit 1 = 1 => datetime available, bit 2 =1 => temperature type available
   * temperature: d3 03 => 03d3 = 979 => 97.9 F
   * year: e5 07 => 07 e5 => 2021
   * month: 07 => 7 => July (0 => month is not known)
   * day: 08 => 8
   * hours: 0f => 15
   * minutes: 22 => 34
   * seconds: 00 => 0
   * type: 01 => ?  body, surface or room (body)
   *
   * example: 07d80300ffe50707090b060001
   * flags: 07, hex to binary = 00000111, bit 0 = 1 => Fahrenheit, bit 1 = 1 => datetime available, bit 2 =1 => temperature type available
   * temperature: d8 03 => 03d8 = 984 => 98.4 F
   * year: e5 07 => 07 e5 => 2021
   * month: 07 => 7 => July (0 => month is not known)
   * day: 09 => 9
   * hours: 0b => 11
   * minutes: 06 => 6
   * seconds: 00 => 0
   * type: 01 => ?  body, surface or room (body)
   *
   * example: 066b0100ffe50707090c150001
   * flags: 06, hex to binary = 00000110, bit 0 = 0 => Celsius, bit 1 = 1 => datetime available, bit 2 =1 => temperature type available
   * temperature: 6b 01 => 016b = 363 => 36.3 V
   * year: e5 07 => 07 e5 => 2021
   * month: 07 => 7 => July (0 => month is not known)
   * day: 09 => 9
   * hours: 0c => 12
   * minutes: 15 => 21
   * seconds: 00 => 0
   * type: 01 => ?  body, surface or room (body)
   *
   * 06 6b 01 00 ff e5 07 07 09 0c 15 00 01
   * 0  1  2  3  4  5  6  7  8  9  10 11 12
   * f  t1 t2 x  x  y1 y2 m  d  h  j  s  t
   *
   */

   // Flags:
   // fahrenheit if bit 0 is on, celcius if off
   // datetime if bit 1 is on, no datetime if off
   // temperature type if bit 2 is on , no type if off
   //
   QBitArray f_arr = QBitArray::fromBits(a.data(),3);
   QString t_format_str = f_arr.at(0) ? "F" : "C";
   if(m_verbose)
   {
     for(int i=0; i<f_arr.size(); i++)
     {
       qDebug() << "flag bit number: " << QString::number(i) << " value: " << (f_arr.at(i) ? "on":"off");
     }
   }

   // Temperature:
   //
   QByteArray t_arr = a.mid(1,2);
   std::reverse(t_arr.begin(), t_arr.end());
   double t_value = t_arr.toHex().toInt(nullptr,16)*0.1;
   QString t_value_str = QString::number(t_value);

   // Datetime:
   //
   QByteArray y_arr = a.mid(5,2);
   std::reverse(y_arr.begin(), y_arr.end());
   int y_value = y_arr.toHex().toInt(nullptr,16);
   QString y_value_str = QString::number(y_value);

   int m_value = a.mid(7,1).toHex().toInt(nullptr,16);
   QString m_value_str = (m_value < 10 ? "0" : "") + QString::number(m_value);

   int d_value = a.mid(8,1).toHex().toInt(nullptr,16);
   QString d_value_str = (d_value < 10 ? "0" : "") + QString::number(d_value);

   int h_value = a.mid(9,1).toHex().toInt(nullptr,16);
   QString h_value_str = (h_value < 10 ? "0" : "") + QString::number(h_value);

   int j_value = a.mid(10,1).toHex().toInt(nullptr,16);
   QString j_value_str = (j_value < 10 ? "0" : "") + QString::number(j_value);

   int s_value = a.mid(11,1).toHex().toInt(nullptr,16);
   QString s_value_str = (s_value < 10 ? "0" : "") + QString::number(s_value);

   // Temperature location type:
   //
   int t_type = a.mid(12,1).toHex().toInt(nullptr,16);
   QString t_type_str = t_type == 1 ? "body" : "surface/room";

   QStringList date_str;
   date_str << y_value_str << m_value_str << d_value_str;
   QStringList time_str;
   time_str << h_value_str << j_value_str << s_value_str;

   if(m_verbose)
       qDebug() << t_value_str << t_format_str << " (" << t_type_str << ") " << date_str.join("-") << " " << time_str.join(":");

   m_measurementData.clear();
   m_measurementData.insert("Temperature Value", QVariant(t_value));
   m_measurementData.insert("Temperature Scale", QVariant(t_format_str));
   m_measurementData.insert("Temperature Type", QVariant(t_type_str));
   m_measurementData.insert("DateTime",QVariant(QDateTime(QDate(y_value,m_value,d_value),QTime(h_value,j_value,s_value))));

   QStringList str;
   str << t_value_str << t_format_str << QString("("+t_type_str+")");
   setProperty("temperature", str.join(" "));

   str.clear();
   str << date_str.join("-") << time_str.join(":");
   setProperty("datetime", str.join(" "));

   disconnectPeripheral();
}
