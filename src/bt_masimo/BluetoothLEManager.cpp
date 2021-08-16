#include "BluetoothLEManager.h"

#include <QMetaEnum>
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

BluetoothLEManager::BluetoothLEManager(QObject *parent) : QObject(parent)
{
}

void BluetoothLEManager::selectAdapter(const QString &address)
{
#ifdef __linux__
    if(!address.isEmpty())
    {
      m_client.reset(new QBluetoothLocalDevice(QBluetoothAddress(address)));
      qDebug() << "using adapter from settings " << address;
    }
    if(m_client.isNull())
    {
      QList<QBluetoothHostInfo> localAdapters = QBluetoothLocalDevice::allDevices();
      if(!localAdapters.empty())
      {
        m_client.reset(new QBluetoothLocalDevice(localAdapters.at(0).address()));
        qDebug() << "using adapter from list of known devices " << m_client->address().toString();
      }
    }

    if(m_client->isValid())
    {
      if(m_client->hostMode()==QBluetoothLocalDevice::HostPoweredOff)
      {
        qDebug() << "local adapter is powered off";
        m_client->powerOn();
      }
      if(m_client->hostMode()!=QBluetoothLocalDevice::HostDiscoverable)
      {
        qDebug() << "setting local adapter host mode to host discoverable";
        m_client->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
      }
    }
    else
    {
        qDebug() << "error: could not find a valid adapter";
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
      qDebug() << "wrote client to settings file";
    }
    if(!m_peripheral.isNull() && m_peripheral->isValid())
    {
      settings->setValue("peripheral/name",m_peripheral->name());
      settings->setValue("peripheral/address",m_peripheral->address().toString());
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
              if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
                  qDebug() << "The Bluetooth adaptor is powered off, power it on before doing discovery.";
              else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
                  qDebug() << "Writing or reading from the device resulted in an error.";
              else
              {
                  static QMetaEnum qme = m_agent->metaObject()->enumerator(
                              m_agent->metaObject()->indexOfEnumerator("Error"));
                  qDebug() << "Error: " << QLatin1String(qme.valueToKey(error));
              }
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
    QString msg = "Found " + QString::number(devices.count()) + " devices";
    qDebug() << msg;

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
  qDebug() << "service discovered "<< uuid.toString();
  if(uuid == QBluetoothUuid(QBluetoothUuid::HealthThermometer))
  {
      qDebug() << "discovered the health thermometer service";
      m_foundThermoService = true;
  }
  else if(uuid == QBluetoothUuid(QBluetoothUuid::DeviceInformation))
  {
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
        qDebug() << "ERROR: no peripheral to search for services" <<
                     (m_peripheralMAC.isEmpty()?"empty peripheral mac " : "peripheral mac ok ") <<
                    (info.isValid()?"device info is valid ":"device info is invalid" ) <<
                    "terminating!";
       // TODO emit fatal error signal
    }

    qDebug() << "ready to connect to " << info.address().toString() << " and discover services";
    clearData();
    if(!m_controller.isNull())
    {
        m_controller->disconnectFromDevice();
    }
    m_peripheral.reset(new QBluetoothDeviceInfo(info));
    m_controller.reset(QLowEnergyController::createCentral(*m_peripheral));
    m_controller->setRemoteAddressType(QLowEnergyController::PublicAddress);

    connect(m_controller.data(), &QLowEnergyController::connected,
      this,[this](){
        qDebug() << "controller finding device services";
        m_controller->discoverServices();
        emit connected();
      }
    );

    connect(m_controller.data(), &QLowEnergyController::disconnected,
      this,[this](){
        emit canConnect();
        qDebug() << "controller disconnected from peripheral";
      }
    );

    connect(m_controller.data(), &QLowEnergyController::serviceDiscovered,
      this, &BluetoothLEManager::serviceDiscovered);

    connect(m_controller.data(), QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error),
      this,[this](QLowEnergyController::Error error)
      {
        qDebug() << "controller error string: " << m_controller->errorString();

        if (error == QLowEnergyController::UnknownError)
           qDebug() << "An unknown error has occurred.";
        else if (error == QLowEnergyController::UnknownRemoteDeviceError)
           qDebug() << "The remote Bluetooth Low Energy device with the address passed to the constructor of this class cannot be found.";
        else if (error == QLowEnergyController::NetworkError)
           qDebug() << "The attempt to read from or write to the remote device failed.";
        else if (error == QLowEnergyController::InvalidBluetoothAdapterError)
           qDebug() << "The local Bluetooth device with the address passed to the constructor of this class cannot be found or there is no local Bluetooth device.";
        else if (error == QLowEnergyController::ConnectionError)
           qDebug() << "The attempt to connect to the remote device failed.";
        else if (error == QLowEnergyController::AdvertisingError)
           qDebug() << "The attempt to start advertising failed.";
        else if (error == QLowEnergyController::RemoteHostClosedError)
           qDebug() << "The remote device closed the connection.";
        else
        {
           static QMetaEnum qme = m_controller->metaObject()->enumerator(
                       m_controller->metaObject()->indexOfEnumerator("Error"));
           qDebug() << "Error: " << QLatin1String(qme.valueToKey(error));
        }
      }
    );

    connect(m_controller.data(), &QLowEnergyController::discoveryFinished,
      this, &BluetoothLEManager::serviceDiscoveryComplete);

    connect(m_controller.data(), &QLowEnergyController::stateChanged,
      this,[](QLowEnergyController::ControllerState state){
        qDebug() << "current controller state: ";
        switch(state)
        {
          case QLowEnergyController::UnconnectedState:
                  qDebug() << "unconnected state";
                  break;
          case QLowEnergyController::ConnectedState:
                  qDebug() << "connected state";
                  break;
          case QLowEnergyController::ConnectingState:
                  qDebug() << "connecting state";
                  break;
          case QLowEnergyController::DiscoveringState:
                  qDebug() << "discovering state";
                  break;
          case QLowEnergyController::DiscoveredState:
                  qDebug() << "discovered state";
                  break;
          case QLowEnergyController::ClosingState:
                  qDebug() << "closing state";
                  break;
          case QLowEnergyController::AdvertisingState:
                  qDebug() << "advertising state";
                  break;
          default: break;
        }
     });

    emit canConnect();
}

void BluetoothLEManager::connectPeripheral()
{
    qDebug() << "slot connectPeripheral called";
    if(!m_controller.isNull() && QLowEnergyController::UnconnectedState == m_controller->state())
    {
        clearData();
        m_controller->connectToDevice();
        qDebug() << "controller connect to device";
    }
}

void BluetoothLEManager::serviceDiscoveryComplete()
{
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
          this, [](const QLowEnergyDescriptor &d, const QByteArray &a)
        {
          qDebug() << ((d.isValid() && a == QByteArray::fromHex("0100"))?"success descriptor write":"descriptor write error");
        }
      );
      m_thermo_service->discoverDetails();
    }
  }
  else
      qDebug() << "health thermometer service not found";

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
      qDebug() << "device information service not found";
}

void BluetoothLEManager::serviceStateChanged(QLowEnergyService::ServiceState state)
{
    auto service = qobject_cast<QLowEnergyService *>(sender());
    qDebug() << "service details discovered for " << service->serviceName();

    switch(state)
    {
      case QLowEnergyService::InvalidService:
        qDebug() << "new state : " << "invalid service";
        break;
      case QLowEnergyService::DiscoveryRequired:
        qDebug() << "new state : " << "discovery required";
        break;
      case QLowEnergyService::DiscoveringServices:
        qDebug() << "new state : " << "discovering services";
        break;
      case QLowEnergyService::ServiceDiscovered:
        qDebug() << "new state : " << "service discovered";
        break;
      case QLowEnergyService::LocalService:
        qDebug() << "new state : " << "local service";
        break;
    }
    if(state != QLowEnergyService::ServiceDiscovered)
        return;

    if("Health Thermometer" == service->serviceName() )
    {
      const QLowEnergyCharacteristic tempChar = service->characteristic(QBluetoothUuid(QBluetoothUuid::TemperatureMeasurement));
      if (!tempChar.isValid())
      {
        qDebug() << "Temperature characteristic invalid.";
        return;
      }

      m_notificationDesc = tempChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
      if(m_notificationDesc.isValid())
      {
          qDebug() << "LE CCC descriptor found ... writing";
          service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
      }
      else
          qDebug() << "failed to write descriptor for health thermometer service";
    }
    else if("Device Information" == service->serviceName())
    {
        const QLowEnergyCharacteristic fwChar = service->characteristic(QBluetoothUuid(QBluetoothUuid::FirmwareRevisionString));
        if (!fwChar.isValid())
        {
          qDebug() << "firmware revision characteristic invalid.";
          return;
        }
        else
        {
          service->readCharacteristic(fwChar);
        }
        const QLowEnergyCharacteristic swChar = service->characteristic(QBluetoothUuid(QBluetoothUuid::SoftwareRevisionString));
        if (!swChar.isValid())
        {
          qDebug() << "software characteristic invalid.";
          return;
        }
        else
        {
          service->readCharacteristic(swChar);
        }
    }
}

void BluetoothLEManager::disconnectPeripheral()
{
     if(!m_measurementData.isEmpty() &&
         m_deviceData.contains("Firmware Revision") &&
         m_deviceData.contains("Software Revision")    )
     {
         qDebug() << "data complete, ready to disconnect";
         qDebug() << "emitting canWrite signal";
         emit canWrite();

         if(QLowEnergyController::DiscoveredState == m_controller->state())
           m_controller->disconnectFromDevice();
     }
}

void BluetoothLEManager::updateInfoData(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    if(c.uuid() == QBluetoothUuid(QBluetoothUuid::FirmwareRevisionString))
    {
        qDebug() << "device info firmware revision update";
        if(c.isValid())
        {
          if(m_deviceData.contains("Firmware Revision"))
             m_deviceData["Firmware Revision"] = QVariant(value);
          else
             m_deviceData.insert("Firmware Revision", QVariant(value));
        }
        else
        {
            if(m_deviceData.contains("Firmware Revision"))
               m_deviceData["Firmware Revision"] = QVariant("uknown");
            else
               m_deviceData.insert("Firmware Revision", QVariant("uknown"));
        }

    }
    else if(c.uuid() == QBluetoothUuid(QBluetoothUuid::SoftwareRevisionString))
    {
        qDebug() << "device info software revision update";
        if(c.isValid())
        {
          if(m_deviceData.contains("Software Revision"))
             m_deviceData["Software Revision"] = QVariant(value);
          else
             m_deviceData.insert("Software Revision", QVariant(value));
        }
        else
        {
            if(m_deviceData.contains("Software Revision"))
               m_deviceData["Software Revision"] = QVariant("uknown");
            else
               m_deviceData.insert("Software Revision", QVariant("uknown"));
        }
    }

    disconnectPeripheral();
}

void BluetoothLEManager::updateTemperatureData(const QLowEnergyCharacteristic &c, const QByteArray &a)
{
  qDebug() << "temperature values update";
  if(c.uuid() != QBluetoothUuid(QBluetoothUuid::TemperatureMeasurement))
  {
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
   for(int i=0; i<f_arr.size(); i++)
   {
       qDebug() << "flag bit number: " << QString::number(i) << " value: " << (f_arr.at(i) ? "on":"off");
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

   qDebug() << t_value_str << t_format_str << " (" << t_type_str << ") " << date_str.join("-") << " " << time_str.join(":");

   m_measurementData.clear();
   m_measurementData.insert("Temperature", QVariant(t_value));
   m_measurementData.insert("Format", QVariant(t_format_str));
   m_measurementData.insert("Type", QVariant(t_type_str));
   m_measurementData.insert("DateTime",QVariant(QDateTime(QDate(y_value,m_value,d_value),QTime(h_value,j_value,s_value))));

   QStringList str;
   str << t_value_str << t_format_str << QString("("+t_type_str+")");
   setProperty("temperature", str.join(" "));

   str.clear();
   str << date_str.join("-") << time_str.join(":");
   setProperty("datetime", str.join(" "));

   disconnectPeripheral();
}
