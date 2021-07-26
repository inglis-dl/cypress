#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QDebug>
#include <QMetaEnum>
#include <QBitArray>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>

#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtBluetooth/QLowEnergyController>

/**
 * Qt 5.14 adds a native Win32 port supporting Classic Bluetooth on Windows 7 or newer, and Bluetooth LE on Windows 8 or newer.
 * It must be enabled at build time by configuration option -native-win32-bluetooth. The UWP backend is used by default if this
 * option is not set and the Win32 target platform supports the required UWP APIs (minimal requirement is Windows 10 version 1507,
 * with slightly improved service discovery since Windows 10 version 1607).
 *
*/

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->scanButton->setEnabled(false);
    ui->clearButton->setEnabled(false);
    ui->connectButton->setEnabled(false);
    ui->saveButton->setEnabled(false);

    readSettings();

    ui->iDlineEdit->setText("40000001"); // dummy ID for now

    // NOTE: QBluetoothLocalDevice is not supported on Windows 7 and 8 targets
    //
    // Verify that the stored local host (client) is the one saved in settings
    // if client is nullptr, find and assign the first local adapter
    //
#ifdef LINUX
    if(nullptr==m_client) {
       QList<QBluetoothHostInfo> localAdapters = QBluetoothLocalDevice::allDevices();
       if(localAdapters.empty()) {
          qDebug() << "failed to find a local adapter";
          close();
       }
       else {
         m_client = new QBluetoothLocalDevice(localAdapters.at(0).address());
         qDebug() << "added local adapter from list";
       }
    }
#endif

    if(!m_peripheralMAC.isEmpty())
    {
      ui->addressLineEdit->setText(m_peripheralMAC);
    }

    // TODO: skip m_deviceData discovery and instantiate the peripheral directly
    // or create a slot for the button click signal to start the agent
    //
    ui->scanButton->setEnabled(true);

    // NOTE: Due to API limitations it is only possible to find devices that have been paired
    // using Windows' settings on Windows.
    //
    m_agent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
          this, &MainWindow::deviceDiscovered);
    connect(m_agent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error),
          this, &MainWindow::deviceScanError);
    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::finished,
          this,[this](){
            QList<QBluetoothDeviceInfo> devices = m_agent->discoveredDevices();
            qDebug() << "Found " << QString::number(devices.count()) << " devices";
            if(nullptr!=m_peripheral)
            {
              ui->connectButton->setEnabled(true);
            }
           });

    m_agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

    connect(ui->saveButton,&QPushButton::clicked, this, &MainWindow::writeMeasurement);
 }

MainWindow::~MainWindow()
{
    delete ui;
    if(nullptr!=m_client) {
      delete m_client;
    }
    if(nullptr!=m_peripheral) {
      delete m_peripheral;
    }
    if(nullptr!=m_agent) {
      delete m_agent;
    }
    if(nullptr!=m_controller) {
      delete m_controller;
    }
}

void MainWindow::readSettings()
{
   QSettings settings(this->m_appDir.filePath("bt_masimo.ini"),QSettings::IniFormat);
   QString address;

#ifdef LINUX
   address = settings.value("client/address").toString();
   if(!address.isEmpty()) {
     m_client = new QBluetoothLocalDevice(QBluetoothAddress(address));

     if(!m_client->isValid()) {
         qDebug() << "client is invalid";
         close();
     }
     m_client->setHostMode(settings.value("client/hostmode").value<QBluetoothLocalDevice::HostMode>());
     qDebug() << "constructed client from settings file";

     if(m_client->hostMode()==QBluetoothLocalDevice::HostPoweredOff)
     {
         qDebug() << "client is powered off";
         m_client->powerOn();
     }
     if(m_client->hostMode()!=QBluetoothLocalDevice::HostDiscoverable)
     {
         qDebug() << "setting client host mode to host discoverable";
         m_client->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
     }
   }
#endif

   // Get the thermometer MAC address
   // If none exists, perform m_deviceData discovery process
   //
   address = settings.value("peripheral/address").toString();
   if(!address.isEmpty()) {
     m_peripheralMAC = address;
     qDebug() << "using peripheral MAC  " << m_peripheralMAC << " from settings file";
   }
}

void MainWindow::writeSettings()
{
   QSettings settings(m_appDir.filePath("bt_masimo.ini"),QSettings::IniFormat);
   if(nullptr!=m_client) {
     settings.setValue("client/name",m_client->name());
     settings.setValue("client/address",m_client->address().toString());
     settings.setValue("client/hostmode",m_client->hostMode());
     qDebug() << "wrote client to settings file";
   }
   if(nullptr!=m_peripheral) {
     settings.setValue("peripheral/name",m_peripheral->name());
     settings.setValue("peripheral/address",m_peripheral->address().toString());
     qDebug() << "wrote peripheral to settings file";
   }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "close event called";
    writeSettings();
    event->accept();
}

void MainWindow::deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    qDebug() << "Found new device:" << info.name() << '(' << info.address().toString() << ')';

    // we can stop the scanning once we find our target m_deviceData
    if(info.address().toString()==m_peripheralMAC) {
        qDebug() << "Found target peripheral with MAC " << info.address().toString() << " ... stopping scan";

        m_peripheral = new QBluetoothDeviceInfo(info);

        m_controller = QLowEnergyController::createCentral(*m_peripheral);

        connect(m_controller, &QLowEnergyController::connected,
                this,&MainWindow::discoverServices);

        connect(m_controller, &QLowEnergyController::disconnected,
                this,[this](){
            ui->connectButton->setEnabled(false);
            qDebug() << "controller disconnected from peripheral";
            });

        connect(m_controller, &QLowEnergyController::serviceDiscovered,
                this, &MainWindow::serviceDiscovered);

        connect(m_controller, QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error),
                this, &MainWindow::serviceScanError);

        connect(m_controller, &QLowEnergyController::discoveryFinished,
                this, &MainWindow::serviceDiscoveryComplete);

        m_controller->setRemoteAddressType(QLowEnergyController::PublicAddress);

        ui->connectButton->setEnabled(true);

        connect(ui->connectButton,&QPushButton::clicked,
            this,[this](){
            qDebug() << "controller connecting to device";
            m_controller->connectToDevice();
            });
    }
}

void MainWindow::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        qDebug() << "The Bluetooth adaptor is powered off, power it on before doing discovery.";
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        qDebug() << "Writing or reading from the device resulted in an error.";
    else {
        static QMetaEnum qme = m_agent->metaObject()->enumerator(
                    m_agent->metaObject()->indexOfEnumerator("Error"));
        qDebug() << "Error: " << QLatin1String(qme.valueToKey(error));
    }
}

void MainWindow::discoverServices()
{
    ui->connectButton->setEnabled(false); // we are connected, so dont allow repeated connect button clicks until we disconnect
    qDebug() << (m_controller->remoteAddressType()==QLowEnergyController::RandomAddress ? "remote address type" : "public address type");
    qDebug() << "controller finding device services";
    m_controller->discoverServices();
}

void MainWindow::serviceDiscovered(const QBluetoothUuid &serviceUuid)
{
  qDebug() << "service discovered ";
  if(serviceUuid == QBluetoothUuid(QBluetoothUuid::HealthThermometer))
  {
      qDebug() << "discovered the health thermometer service";
      foundThermometer = true;
  }
  else if(serviceUuid == QBluetoothUuid(QBluetoothUuid::DeviceInformation))
  {
     qDebug() << "discovered the device information service";
     foundDeviceInfo = true;
  }
}

void MainWindow::serviceDiscoveryComplete()
{
  qDebug() << "controller service discovery complete";

  if(!foundThermometer)
  {
      qDebug() << "error: did not discover the health thermometer service";
      return;
  }

  QLowEnergyService *h_service = m_controller->createServiceObject(QBluetoothUuid(QBluetoothUuid::HealthThermometer), m_controller);
  if (!h_service) {
      qDebug() << "Cannot create service for thermometer";
      return;
  }
  qDebug() << "Health Thermometer " << (QLowEnergyService::PrimaryService == h_service->type() ? "Primary" : "Included") << " service type";

  connect(h_service, &QLowEnergyService::stateChanged, this, &MainWindow::serviceDetailsState);

  connect(h_service, &QLowEnergyService::characteristicChanged, this, &MainWindow::updateTemperatureValue);

  connect(h_service, &QLowEnergyService::descriptorWritten, this, &MainWindow::confirmedDescriptorWrite);

  h_service->discoverDetails();

  // get the device information
  if(!foundDeviceInfo)
  {
      qDebug() << "error: did not discover the device information service";
      return;
  }
  QLowEnergyService *d_service = m_controller->createServiceObject(QBluetoothUuid(QBluetoothUuid::DeviceInformation), m_controller);
  if (!d_service) {
      qDebug() << "Cannot create service for thermometer device information";
      return;
  }
  qDebug() << "Device Information " <<  (QLowEnergyService::PrimaryService == d_service->type() ? "Primary" : "Included") << " service type";
  connect(d_service, &QLowEnergyService::stateChanged, this, &MainWindow::serviceDetailsState);
  d_service->discoverDetails();

}

void MainWindow::confirmedDescriptorWrite(const QLowEnergyDescriptor& d, const QByteArray& a)
{
   qDebug() << "confirmed descriptor write with value: " << BLEInfo::valueToString(a);

   if(d.isValid() && a == QByteArray::fromHex("0100"))
   {
       qDebug() << "success write";
   }
   else
   {
       qDebug() << "write error";
   }
}

void MainWindow::serviceDetailsState(QLowEnergyService::ServiceState newState)
{
    if(QLowEnergyService::InvalidService == newState)
      qDebug() << "new state : " << "invalid service";
    if(QLowEnergyService::DiscoveryRequired == newState)
      qDebug() << "new state : " << "discovery required";
    if(QLowEnergyService::DiscoveringServices == newState)
      qDebug() << "new state : " << "discovering services";
    if(QLowEnergyService::ServiceDiscovered == newState)
      qDebug() << "new state : " << "service discovered";
    if(QLowEnergyService::LocalService == newState)
      qDebug() << "new state : " << "local service";
    if (newState != QLowEnergyService::ServiceDiscovered) {
/*
        // do not hang in "Scanning for characteristics" mode forever
        // in case the service discovery failed
        // We have to queue the signal up to give UI time to even enter
        // the above mode
        if (newState != QLowEnergyService::DiscoveringServices) {
            QMetaObject::invokeMethod(this, "characteristicsUpdated",
                                      Qt::QueuedConnection);
        }
*/
        return;
    }


    auto service = qobject_cast<QLowEnergyService *>(sender());
    if (!service)
    {
        qDebug() << "error: failed to create LE service from sender";
        return;
    }

    qDebug() << "service details discovered for " << service->serviceName();
    if("Health Thermometer" == service->serviceName() )
    {
      const QLowEnergyCharacteristic tempChar = service->characteristic(QBluetoothUuid(QBluetoothUuid::TemperatureMeasurement));
      if (!tempChar.isValid())
      {
        qDebug() << "Temperature characteristic invalid.";
        return;
      }

      QLowEnergyDescriptor desc = tempChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
      if (desc.isValid())
      {
          qDebug() << "LE CCC descriptor found ... writing";
          service->writeDescriptor(desc, QByteArray::fromHex("0100"));
      }
    }
    else if("Device Information" == service->serviceName())
    {
        const QLowEnergyCharacteristic frvChar = service->characteristic(QBluetoothUuid(QBluetoothUuid::FirmwareRevisionString));
        const QLowEnergyCharacteristic swChar = service->characteristic(QBluetoothUuid(QBluetoothUuid::SoftwareRevisionString));
        if(frvChar.isValid())
        {
            m_deviceData.clear();
            m_deviceData.insert("Device Name",QVariant(m_peripheral->name()));
            m_deviceData.insert("Device MAC",QVariant(m_peripheral->address().toString()));
            m_deviceData.insert("Firmware Revision", QVariant(frvChar.value()));
            m_deviceData.insert("Software Revision", QVariant(swChar.value()));
        }

        /**
           // Optional characteristics that are correctly observed by the Masimo TR-1 thermometer
           // Firmware Revision String
           // Software Revision String
           // PnP ID
           // "The PnP_ID characteristic is a set of values that used to create a m_deviceData ID value that is unique for this m_deviceData.
           // Included in the characteristic is a Vendor ID Source field, a Vendor ID field, a Product ID field and a Product Version field.
           // These values are used to identify all devices of a given type/model/version using numbers."
           //
           // Vendor ID Source uint8 key =1 => "Bluetooth SIG assigned Company Identifier value from the Assigned Numbers document"
           //   key = 2 => "USB Implementer’s Forum assigned Vendor ID value"
           //   Vendor ID uint16  LSO to MSO
           //   Product ID unit16 LSO to MSO
           //   Product Version uint16 LSO to MSO
           // PnP ID is not valid for the Masimo TR-1
       */
    }
}

void MainWindow::updateTemperatureValue(const QLowEnergyCharacteristic &c, const QByteArray& a)
{
  qDebug() << "temperature value update";
  if (c.uuid() != QBluetoothUuid(QBluetoothUuid::TemperatureMeasurement) || a.isEmpty())
  {
      qDebug() << "update temperature error: wrong characteristic or empty data";
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
   * */

   // flags:
   QBitArray f_arr = QBitArray::fromBits(a.data(),3);
   QString t_format_str = f_arr.at(0) ? "F" : "C";
   for(int i=0;i<f_arr.size();i++)
   {
       qDebug() << "flag bit number: " << QString::number(i) << " value: " << (f_arr.at(i) ? "on":"off");
   }
   // fahrenheit if bit 0 is on, celcius if off
   // datetime if bit 1 is on, no datetime if off
   // temperature type if bit 2 is on , no type if off

   // temperature:
   QByteArray t_arr = a.mid(1,2);
   std::reverse(t_arr.begin(), t_arr.end());
   double t_value = t_arr.toHex().toInt(nullptr,16)*0.1;
   QString t_value_str = QString::number(t_value);

   // 06 6b 01 00 ff e5 07 07 09 0c 15 00 01
   // 0  1  2  3  4  5  6  7  8  9  10 11 12
   // f  t1 t2 x  x  y1 y2 m  d  h  j  s  t

   // datetime:
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
   str << t_value_str << t_format_str;
   ui->temperatureLineEdit->setText(str.join(" "));
   str.clear();
   str << date_str << time_str;
   ui->dateTimeLineEdit->setText(str.join(" "));
   ui->saveButton->setEnabled(true);
}

void MainWindow::serviceScanError(QLowEnergyController::Error error)
{
     qDebug() << "controller error string: " << m_controller->errorString();

    if (error == QLowEnergyController::UnknownError)
        qDebug() << "An unknown error has occurred.";
    else if (error == QLowEnergyController::UnknownRemoteDeviceError)
        qDebug() << "The remote Bluetooth Low Energy m_deviceData with the address passed to the constructor of this class cannot be found.";
    else if (error == QLowEnergyController::NetworkError)
        qDebug() << "The attempt to read from or write to the remote m_deviceData failed.";
    else if (error == QLowEnergyController::InvalidBluetoothAdapterError)
        qDebug() << "The local Bluetooth m_deviceData with the address passed to the constructor of this class cannot be found or there is no local Bluetooth m_deviceData.";
    else if (error == QLowEnergyController::ConnectionError)
        qDebug() << "The attempt to connect to the remote m_deviceData failed.";
    else if (error == QLowEnergyController::AdvertisingError)
        qDebug() << "The attempt to start advertising failed.";
    else if (error == QLowEnergyController::RemoteHostClosedError)
        qDebug() << "The remote m_deviceData closed the connection.";
    else {
        static QMetaEnum qme = m_controller->metaObject()->enumerator(
                    m_controller->metaObject()->indexOfEnumerator("Error"));
        qDebug() << "Error: " << QLatin1String(qme.valueToKey(error));
    }
}

void MainWindow::writeMeasurement()
{
   qDebug() << "begin write process ... ";

   // Create a json object with measurement key value pairs
   //
   QJsonObject json;
   QMap<QString,QVariant>::const_iterator it = m_measurementData.constBegin();
   while(it != m_measurementData.constEnd()) {
     json.insert(it.key(),QJsonValue::fromVariant(it.value()));
     ++it;
   }
   it = m_deviceData.constBegin();
   while(it != m_deviceData.constEnd()) {
     json.insert(it.key(),QJsonValue::fromVariant(it.value()));
     ++it;
   }

   // Insert participant barcode
   //
   QString barcode = ui->iDlineEdit->text().simplified().remove(" ");
   json.insert("Barcode",QJsonValue(barcode));

   // Output .json file will be of the form <participant ID>_<now>_<devicename>.json
   //
   QStringList jsonFile;
   jsonFile << barcode;
   jsonFile << QDate().currentDate().toString("yyyyMMdd");
   jsonFile << "bt_masimo.json";
   QFile saveFile( m_appDir.filePath( jsonFile.join("_") ) );
   saveFile.open(QIODevice::WriteOnly);
   saveFile.write(QJsonDocument(json).toJson());

   qDebug() << "wrote to file " << m_appDir.filePath( jsonFile.join("_") );
}

QString BLEInfo::uuidToString(const QBluetoothUuid& uuid)
{
    bool success = false;
    quint16 result16 = uuid.toUInt16(&success);
    if (success)
        return QStringLiteral("0x") + QString::number(result16, 16);

    quint32 result32 = uuid.toUInt32(&success);
    if (success)
        return QStringLiteral("0x") + QString::number(result32, 16);

    return uuid.toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
}

QString BLEInfo::valueToString(const QByteArray& a)
{
    // Show raw string first and hex value below
    QString result;
    if (a.isEmpty()) {
        result = QStringLiteral("<none>");
        return result;
    }

    result = a;
    result += QLatin1Char('\n');
    result += a.toHex();

    return result;
}

QString BLEInfo::handleToString(const QLowEnergyHandle& h)
{
    return QStringLiteral("0x") + QString::number(h, 16);
}

QString BLEInfo::permissionToString(const QLowEnergyCharacteristic& c)
{
    QString properties = "( ";
    uint permission = c.properties();
    if (permission & QLowEnergyCharacteristic::Read)
        properties += QStringLiteral(" Read");
    if (permission & QLowEnergyCharacteristic::Write)
        properties += QStringLiteral(" Write");
    if (permission & QLowEnergyCharacteristic::Notify)
        properties += QStringLiteral(" Notify");
    if (permission & QLowEnergyCharacteristic::Indicate)
        properties += QStringLiteral(" Indicate");
    if (permission & QLowEnergyCharacteristic::ExtendedProperty)
        properties += QStringLiteral(" ExtendedProperty");
    if (permission & QLowEnergyCharacteristic::Broadcasting)
        properties += QStringLiteral(" Broadcast");
    if (permission & QLowEnergyCharacteristic::WriteNoResponse)
        properties += QStringLiteral(" WriteNoResp");
    if (permission & QLowEnergyCharacteristic::WriteSigned)
        properties += QStringLiteral(" WriteSigned");
    properties += " )";
    return properties;
}
