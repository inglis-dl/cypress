#include "BluetoothLEManager.h"

#include <QBitArray>
#include <QDateTime>
#include <QtBluetooth/QBluetoothHostInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>
#include <QStandardItemModel>

/**
 * Qt 5.14 adds a native Win32 port supporting Classic Bluetooth on Windows 7 or newer, and Bluetooth LE on Windows 8 or newer.
 * It must be enabled at build time by configuration option -native-win32-bluetooth. The UWP backend is used by default if this
 * option is not set and the Win32 target platform supports the required UWP APIs (minimal requirement is Windows 10 version 1507,
 * with slightly improved service discovery since Windows 10 version 1607).
 *
*/

BluetoothLEManager::BluetoothLEManager(QObject *parent) : ManagerBase(parent)
{
    setGroup("thermometer");
    m_col = 1;
    m_row = 2;

    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode";
    m_inputKeyList << "language";

    m_test.setExpectedMeasurementCount(2);
}

void BluetoothLEManager::start()
{
    scanDevices();
    emit dataChanged();
}

void BluetoothLEManager::buildModel(QStandardItemModel *model) const
{
    for(int row = 0; row < m_row; row++)
    {
        QString s = "NA";
        TemperatureMeasurement m = m_test.getMeasurement(row);
        if(m.isValid())
           s = m.toString();
        QStandardItem* item = model->item(row,0);
        item->setData(s, Qt::DisplayRole);
    }
}

void BluetoothLEManager::setLocalDevice(const QString &address)
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
      return;
    }
#ifdef Q_OS_LINUX
    if(!address.isEmpty())
    {
      m_localDevice.reset(new QBluetoothLocalDevice(QBluetoothAddress(address)));
      if(m_verbose)
          qDebug() << "using local adapter from settings " << address;
    }
    if(m_localDevice.isNull())
    {
      QList<QBluetoothHostInfo> localAdapters = QBluetoothLocalDevice::allDevices();
      if(!localAdapters.empty())
      {
        m_localDevice.reset(new QBluetoothLocalDevice(localAdapters.at(0).address()));
        if(m_verbose)
            qDebug() << "using first local adapter from list of known devices " << m_localDevice->address().toString();
      }
    }

    if(m_localDevice->isValid())
    {
      if(QBluetoothLocalDevice::HostPoweredOff == m_localDevice->hostMode())
      {
        if(m_verbose)
            qDebug() << "local adapter is powered off: powering on";
        m_localDevice->powerOn();
      }
      if(QBluetoothLocalDevice::HostDiscoverable != m_localDevice->hostMode())
      {
        if(m_verbose)
            qDebug() << "setting local adapter host mode to host discoverable";
        m_localDevice->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
      }
    }
    else
    {
      if(m_verbose)
        qDebug() << "error: could not find a local adapter";
    }
 #endif
}

void BluetoothLEManager::loadSettings(const QSettings &settings)
{
#ifdef Q_OS_LINUX
    QString localAddress = settings.value(getGroup() + "/client/address").toString();
    qDebug() << "setting local device from settings file";
    setLocalDevice(localAddress);
#endif

    // Get the thermometer MAC address.
    // If none exists perform device discovery process
    //
    QString address = settings.value(getGroup() + "/peripheral/address").toString();
    if(!address.isEmpty())
    {
      setProperty("deviceName", address);
      if(m_verbose)
          qDebug() << "using peripheral MAC " << m_deviceName << " from settings file";
    }
}

void BluetoothLEManager::saveSettings(QSettings *settings) const
{
    if(!m_localDevice.isNull() && m_localDevice->isValid())
    {
      settings->beginGroup(getGroup());
      settings->setValue("client/name",m_localDevice->name());
      settings->setValue("client/address",m_localDevice->address().toString());
      settings->setValue("client/hostmode",m_localDevice->hostMode());
      settings->endGroup();
      if(m_verbose)
          qDebug() << "wrote local adapter to settings file";
    }
    if(!m_peripheral.isNull() && m_peripheral->isValid())
    {
      settings->beginGroup(getGroup());
      settings->setValue("peripheral/name",m_peripheral->name());
      settings->setValue("peripheral/address",m_peripheral->address().toString());
      settings->endGroup();
      if(m_verbose)
          qDebug() << "wrote peripheral device to settings file";
    }
}

void BluetoothLEManager::setInputData(const QVariantMap& input)
{
    m_inputData = input;
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        if(!input.contains("barcode"))
          m_inputData["barcode"] = Constants::DefaultBarcode;
        if(!input.contains("language"))
          m_inputData["language"] = "en";
    }
    bool ok = true;
    QMap<QString,QMetaType::Type> typeMap {
        {"barcode",QMetaType::Type::QString},
        {"language",QMetaType::Type::QString}
    };
    foreach(const auto key, m_inputKeyList)
    {
      if(!m_inputData.contains(key))
      {
        ok = false;
        if(m_verbose)
          qDebug() << "ERROR: missing expected input " << key;
        break;
      }
      const QVariant value = m_inputData[key];
      bool valueOk = true;
      QMetaType::Type type;
      if(typeMap.contains(key))
      {
        type = typeMap[key];
        valueOk = value.canConvert(type);
      }
      if(!valueOk)
      {
        ok = false;
        if(m_verbose)
          qDebug() << "ERROR: invalid input" << key << value.toString() << QMetaType::typeName(type);
        break;
      }
    }
    if(!ok)
    {
      if(m_verbose)
        qDebug() << "ERROR: invalid input data";

      emit message(tr("ERROR: the input data is incorrect"));
      m_inputData = QVariantMap();
    }
}

bool BluetoothLEManager::lowEnergyEnabled() const
{
  if(Constants::RunMode::modeSimulate == m_mode)
  {
      return true;
  }
  return (QBluetoothDeviceDiscoveryAgent::LowEnergyMethod &
          QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods());
}

bool BluetoothLEManager::localDeviceEnabled() const
{
    bool enabled = true;
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        return enabled;
    }
#ifdef Q_OS_LINUX
    enabled = (!m_localDevice.isNull() && m_localDevice->isValid());
#endif
    return enabled;
}

void BluetoothLEManager::scanDevices()
{
    m_deviceList.clear();
    emit scanningDevices();
    if(m_verbose)
      qDebug() << "start scanning for devices ....";

    if(Constants::RunMode::modeSimulate == m_mode)
    {
      QBluetoothAddress address("89:CC:44:EB:03:14");
      QString name = "simulated_device";
      quint32 deviceClass = 0;
      QBluetoothDeviceInfo info(address,name,deviceClass);
      QString label = QString("%1 %2").arg(name,info.address().toString());
      m_deviceList.insert(label,info);
      emit deviceDiscovered(label);
      if(m_verbose)
        qDebug() << "found device " << label;
      emit deviceSelected(label);
      setDevice(info);
      return;
    }

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
           this, &BluetoothLEManager::deviceDiscoveredInternal);

      connect(m_agent.data(), QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error),
            this,[this](QBluetoothDeviceDiscoveryAgent::Error error)
        {
          QStringList s = QVariant::fromValue(error).toString().split(QRegExp("(?=[A-Z])"),Qt::SkipEmptyParts);
          if(m_verbose)
              qDebug() << "device discovery error " << s.join(" ").toLower();
        }
      );

      connect(m_agent.data(), &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothLEManager::discoveryCompleteInternal);

      connect(m_agent.data(), &QBluetoothDeviceDiscoveryAgent::canceled,
              this, &BluetoothLEManager::discoveryCompleteInternal);
    }

    m_agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

bool BluetoothLEManager::isPairedTo(const QString &label) const
{
    bool paired = false;
    if(m_deviceList.contains(label) && !m_localDevice.isNull())
    {
        QBluetoothDeviceInfo info = m_deviceList.value(label);
        QBluetoothLocalDevice::Pairing pairingStatus = m_localDevice->pairingStatus(info.address());
        paired = (pairingStatus == QBluetoothLocalDevice::Paired || pairingStatus == QBluetoothLocalDevice::AuthorizedPaired);
    }
    return paired;
}

// add a device to the ui list
//
void BluetoothLEManager::deviceDiscoveredInternal(const QBluetoothDeviceInfo &info)
{
    if(m_verbose)
    {
        qDebug() << "Found device:" << info.name() << '(' << info.address().toString() << ')';
        QBluetoothDeviceInfo::CoreConfigurations config = info.coreConfigurations();
        if(QBluetoothDeviceInfo::UnknownCoreConfiguration & config)
        {
            qDebug() << "unknown core configuration";
        }
        if(QBluetoothDeviceInfo::BaseRateCoreConfiguration & config)
        {
            qDebug() << "base rate core configuration";
        }
        if(QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration & config)
        {
            qDebug() << "base rate and core configuration";
        }
        if(QBluetoothDeviceInfo::LowEnergyCoreConfiguration & config)
        {
            qDebug() << "low energy core configuration";
        }

        QString services;
        if(info.serviceClasses() & QBluetoothDeviceInfo::PositioningService)
            services += "Position|";
        if(info.serviceClasses() & QBluetoothDeviceInfo::NetworkingService)
            services += "Network|";
        if(info.serviceClasses() & QBluetoothDeviceInfo::RenderingService)
            services += "Rendering|";
        if(info.serviceClasses() & QBluetoothDeviceInfo::CapturingService)
            services += "Capturing|";
        if(info.serviceClasses() & QBluetoothDeviceInfo::ObjectTransferService)
            services += "ObjectTra|";
        if(info.serviceClasses() & QBluetoothDeviceInfo::AudioService)
            services += "Audio|";
        if(info.serviceClasses() & QBluetoothDeviceInfo::TelephonyService)
            services += "Telephony|";
        if(info.serviceClasses() & QBluetoothDeviceInfo::InformationService)
            services += "Information|";

        services.truncate(services.length()-1); //cut last '/'

        qDebug() << "Found new device: " << info.name() << info.isValid() << info.address().toString()
                                         << info.rssi() << info.majorDeviceClass()
                                         << info.minorDeviceClass() << services;
    }

    // Add the device to the list
    //
    QString label = QString("%1 %2").arg(info.name(),info.address().toString());
    if(!m_deviceList.contains(label))
    {
        m_deviceList.insert(label,info);
        emit deviceDiscovered(label);

        if(m_deviceName == info.address().toString() && m_agent->isActive())
        {
            if(m_verbose)
              qDebug() << "stopping discovery agent, found the device "<< m_deviceName;
            m_agent->stop();
        }
    }
}

// Enable discovered devices selection
//
void BluetoothLEManager::discoveryCompleteInternal()
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
    QString label;
    QBluetoothDeviceInfo info;
    if(!m_deviceName.isEmpty())
    {
      qDebug() << "searching discovery list for previously found device name: " << m_deviceName;
      QMap<QString,QBluetoothDeviceInfo>::const_iterator it = m_deviceList.constBegin();
      while(it != m_deviceList.constEnd() && !found)
      {
        label = it.key();
        found = it.key().contains(m_deviceName);
        if(found) info = it.value();
        ++it;
      }
    }
    if(found)
    {
      if(m_verbose)
        qDebug() << "found the peripheral with mac address " << m_deviceName;

      emit deviceSelected(label);
      setDevice(info);
    }
    else
    {
      // select a peripheral from the list of scanned devices
      emit message(tr("Ready to select..."));
      emit canSelectDevice();
    }
}

// select the bluetooth peripheral device
//
void BluetoothLEManager::selectDevice(const QString &label)
{
    if(m_deviceList.contains(label))
    {
      QBluetoothDeviceInfo info = m_deviceList.value(label);
      setProperty("deviceName",info.address().toString());
      setDevice(info);
      if(m_verbose)
        qDebug() << "device selected from list " <<  label;
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
  }
  else if(uuid == QBluetoothUuid(QBluetoothUuid::DeviceInformation))
  {
    if(m_verbose)
      qDebug() << "discovered the device information service";
  }
}

void BluetoothLEManager::clearData()
{
    m_deviceData.reset();
    m_test.reset();
    emit dataChanged();
}

void BluetoothLEManager::finish()
{
    if(!m_controller.isNull() && QLowEnergyController::UnconnectedState != m_controller->state())
    {
      m_controller->disconnectFromDevice();
    }
    m_deviceData.reset();
    m_test.reset();
}

void BluetoothLEManager::setDevice(const QBluetoothDeviceInfo &info)
{
    clearData();
    if(Constants::RunMode::modeSimulate == m_mode)
    {
       // get the device data
       m_deviceData.setAttribute("device_name", info.name());
       m_deviceData.setAttribute("device_mac_address", info.address().toString());
       emit canConnectDevice();
       return;
    }

    // sanity check
    //
    if(m_deviceName.isEmpty() || !info.isValid())
    {
        if(m_verbose)
            qDebug() << "ERROR: no peripheral to search for services" <<
                     (m_deviceName.isEmpty()?"empty peripheral mac " : "peripheral mac ok ") <<
                    (info.isValid()?"device info is valid ":"device info is invalid" ) <<
                    "terminating!";
       // TODO emit fatal error signal
    }
    if(m_verbose)
        qDebug() << "ready to connect to " << info.address().toString() << " and discover services";

    if(!m_controller.isNull())
    {
        m_controller->disconnectFromDevice();
    }

    m_deviceData.setAttribute("device_name", info.name());
    m_deviceData.setAttribute("device_mac_address", info.address().toString());

    m_peripheral.reset(new QBluetoothDeviceInfo(info));
    m_controller.reset(QLowEnergyController::createCentral(*m_peripheral));
    m_controller->setRemoteAddressType(QLowEnergyController::PublicAddress);

    connect(m_controller.data(), &QLowEnergyController::disconnected,
      this,[this](){
        if(m_verbose)
            qDebug() << "controller disconnected from " << m_controller->remoteName();
        emit message(tr("Ready to connect..."));
        emit canConnectDevice();
      }
    );

    connect(m_controller.data(), &QLowEnergyController::serviceDiscovered,
      this, &BluetoothLEManager::serviceDiscovered);

    connect(m_controller.data(), QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error),
      this,[this](QLowEnergyController::Error error)
      {
        QStringList s = QVariant::fromValue(error).toString().split(QRegExp("(?=[A-Z])"),Qt::SkipEmptyParts);
        if(m_verbose)
          qDebug() << "low energy controller error " << s.join(" ").toLower();
      }
    );

    connect(m_controller.data(), &QLowEnergyController::discoveryFinished,
      this, &BluetoothLEManager::serviceDiscoveryComplete);

    connect(m_controller.data(), &QLowEnergyController::stateChanged,
      this,[this](QLowEnergyController::ControllerState state){
        QStringList s = QVariant::fromValue(state).toString().split(QRegExp("(?=[A-Z])"),Qt::SkipEmptyParts);
        if(m_verbose)
          qDebug() << "controller state changed to " << s.join(" ").toLower();
     });

    connect(m_controller.data(), &QLowEnergyController::connected,
            this,[this](){
        if(m_verbose)
            qDebug() <<  "controller finding " << m_controller->remoteName() << " services";
        m_controller->discoverServices();
    });

    emit message(tr("Ready to connect..."));
    emit canConnectDevice();
}

void BluetoothLEManager::connectDevice()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        emit message(tr("Ready to measure..."));
        emit canMeasure();
        return;
    }
    if(!m_controller.isNull() && QLowEnergyController::UnconnectedState == m_controller->state())
    {
        if(m_verbose)
          qDebug() << "controller connecting to device";
        m_controller->connectToDevice();
    }
}

void BluetoothLEManager::serviceDiscoveryComplete()
{
  if(m_verbose)
  {
    qDebug() << "controller service discovery complete";

    foreach(const auto x, m_controller->services())
    {
      qDebug() << x.toString();
      quint16 uid = x.toUInt16();
      qDebug() << QBluetoothUuid::serviceClassToString(static_cast<QBluetoothUuid::ServiceClassUuid>(uid));
    }
  }

  m_thermo_service.reset(
    m_controller->createServiceObject(QBluetoothUuid(QBluetoothUuid::HealthThermometer),
    m_controller.data()));
  if(!m_thermo_service.isNull())
  {
      connect(m_thermo_service.data(), &QLowEnergyService::stateChanged,
          this, &BluetoothLEManager::thermoServiceStateChanged);
      connect(m_thermo_service.data(), &QLowEnergyService::characteristicChanged,
          this, &BluetoothLEManager::updateTemperatureData);
      connect(m_thermo_service.data(), &QLowEnergyService::descriptorWritten,
          this,[this](const QLowEnergyDescriptor &d, const QByteArray &a)
        {
          if(m_verbose)
          {
              if(d.isValid())
              {
                if(a == QByteArray::fromHex("0200"))
                  qDebug() << "success write for indicate";
                else if(a == QByteArray::fromHex("0000"))
                  qDebug() << "success write for reset no indicate";
                else
                  qDebug() << "descriptor write error";
              }
              else
                qDebug() << "invalid descriptor written";
          }
        });
      m_thermo_service->discoverDetails();
  }
  else
  {
    if(m_verbose)
      qDebug() << "error: expected health thermometer service not found";
  }

  m_info_service.reset(
    m_controller->createServiceObject(QBluetoothUuid(QBluetoothUuid::DeviceInformation),
    m_controller.data()));
  if(!m_info_service.isNull())
  {
      connect(m_info_service.data(), &QLowEnergyService::stateChanged,
        this, &BluetoothLEManager::infoServiceStateChanged);
      connect(m_info_service.data(), &QLowEnergyService::characteristicRead,
        this, &BluetoothLEManager::updateInfoData);
      m_info_service->discoverDetails();
  }
  else
  {
    if(m_verbose)
      qDebug() << "error: expected device information service not found";
  }
}

void BluetoothLEManager::measure()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        m_deviceData.setAttribute("device_firmware_revision", "1.0.0");
        m_deviceData.setAttribute("device_software_revision", "1.0.0");
        m_test.addMeasurement(TemperatureMeasurement::simulate(Constants::UnitsSystem::systemMetric));
        if(m_test.isValid())
        {
          emit message(tr("Ready to save results..."));
          emit canWrite();
        }
        emit dataChanged();
        return;
    }

    const QLowEnergyCharacteristic firmwareChar = m_info_service->characteristic(QBluetoothUuid(QBluetoothUuid::FirmwareRevisionString));
    if(firmwareChar.isValid())
    {
      m_info_service->readCharacteristic(firmwareChar);
    }
    else
      qDebug() << "error: invalid firmware characteristic, unable to read";

    const QLowEnergyCharacteristic softwareChar = m_info_service->characteristic(QBluetoothUuid(QBluetoothUuid::SoftwareRevisionString));
    if(softwareChar.isValid())
    {
      m_info_service->readCharacteristic(softwareChar);
    }
    else
      qDebug() << "error: invalid software characteristic, unable to read";

    const QLowEnergyCharacteristic temperatureChar = m_thermo_service->characteristic(QBluetoothUuid(QBluetoothUuid::TemperatureMeasurement));
    if(temperatureChar.isValid())
    {
        QString properties;
        if(temperatureChar.properties() & QLowEnergyCharacteristic::Unknown)
            properties += "Unknown|";
        if(temperatureChar.properties() & QLowEnergyCharacteristic::Broadcasting)
            properties += "Broadcasting|";
        if(temperatureChar.properties() & QLowEnergyCharacteristic::Read)
            properties += "Read|";
        if(temperatureChar.properties() & QLowEnergyCharacteristic::WriteNoResponse)
            properties += "WriteNoResponse|";
        if(temperatureChar.properties() & QLowEnergyCharacteristic::Write)
            properties += "Write|";
        if(temperatureChar.properties() & QLowEnergyCharacteristic::Notify)
            properties += "Notify|";
        if(temperatureChar.properties() & QLowEnergyCharacteristic::Indicate)
            properties += "Indicate|";
        if(temperatureChar.properties() & QLowEnergyCharacteristic::WriteSigned)
            properties += "WriteSigned|";
        if(temperatureChar.properties() & QLowEnergyCharacteristic::ExtendedProperty)
            properties += "ExtendedProperty|";

        properties.truncate(properties.length()-1); //cut last '/'
        qDebug() << "temperature characteristic properties: " << properties;

        QLowEnergyDescriptor cccDesc = temperatureChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if(cccDesc.isValid())
        {
          if(m_verbose)
            qDebug() << cccDesc.name() << " found ... writing";
          m_thermo_service->writeDescriptor(cccDesc, QByteArray::fromHex("0200"));
        }
     }
    else
      qDebug() << "error: invalid temperature characteristic, cannot write descriptor for update and read";
}

void BluetoothLEManager::infoServiceStateChanged(QLowEnergyService::ServiceState state)
{
    auto service = qobject_cast<QLowEnergyService *>(sender());
    QString name = service->serviceName();
    QStringList s = QVariant::fromValue(state).toString().split(QRegExp("(?=[A-Z])"),Qt::SkipEmptyParts);
    if(m_verbose)
      qDebug() << name << " service state changed to " << s.join(" ").toLower();;

    if(QLowEnergyService::ServiceDiscovered != state)
      return;

    // if both services have been discovered emit the canMeasure signal
    //
    if(QLowEnergyService::ServiceDiscovered == m_thermo_service->state())
    {
      emit message(tr("Ready to measure..."));
      emit canMeasure();
    }
}

void BluetoothLEManager::thermoServiceStateChanged(QLowEnergyService::ServiceState state)
{
    auto service = qobject_cast<QLowEnergyService *>(sender());
    QString name = service->serviceName();
    QStringList s = QVariant::fromValue(state).toString().split(QRegExp("(?=[A-Z])"),Qt::SkipEmptyParts);
    if(m_verbose)
        qDebug() << name << " service state changed to " << s.join(" ").toLower();;

    if(QLowEnergyService::ServiceDiscovered != state)
        return;

    // if both services have been discovered emit the canMeasure signal
    //
    if(QLowEnergyService::ServiceDiscovered == m_info_service->state())
    {
        emit message(tr("Ready to measure..."));
        emit canMeasure();
    }
}

void BluetoothLEManager::disconnectDevice()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        emit message("Ready to connect...");
        emit canConnectDevice();
        return;
    }
    if(!m_controller.isNull())
    {
        m_controller->disconnectFromDevice();
    }
}

void BluetoothLEManager::updateInfoData(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    if(c.uuid() == QBluetoothUuid(QBluetoothUuid::FirmwareRevisionString))
    {
      if(m_verbose)
          qDebug() << "device info firmware revision update";
      if(c.isValid())
      {
        if(m_verbose)
          qDebug() << "OK: setting firmware revision value " <<  QString(value);
        m_deviceData.setAttribute("device_firmware_revision", QString(value));
      }
    }
    else if(c.uuid() == QBluetoothUuid(QBluetoothUuid::SoftwareRevisionString))
    {
      if(m_verbose)
          qDebug() << "device info software revision update";
      if(c.isValid())
      {
        if(m_verbose)
          qDebug() << "OK: setting sofware revision value " <<  QString(value);
        m_deviceData.setAttribute("device_software_revision", QString(value));
      }
    }
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
   qDebug() << "adding data to test " << QString(arr);
   m_test.fromArray(arr);
   if(m_test.isValid())
   {
     emit message(tr("Ready to save results..."));
     emit canWrite();
   }
   emit dataChanged();

   QLowEnergyDescriptor cccDesc = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
   if(cccDesc.isValid())
   {
       if(m_verbose)
             qDebug() << cccDesc.name() << " found ... writing a reset";
        m_thermo_service->writeDescriptor(cccDesc, QByteArray::fromHex("0000"));
   }
   else
       qDebug() << "update temperature data invalid descriptor";
}

QJsonObject BluetoothLEManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    json.insert("device",m_deviceData.toJsonObject());
    json.insert("test_input",QJsonObject::fromVariantMap(m_inputData));
    return json;
}
