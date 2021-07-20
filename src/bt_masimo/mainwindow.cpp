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

// TODO have the user enter the mac address of the thermometer found on the label on
// the underside of the device
//
const QString peripheralMAC = "C0:26:DA:13:B0:DF";

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

    ui->iDlineEdit->setText("30000001"); // dummy ID for now

    // verify that the stored local host (client) is the one saved in settings
    // if client is nullptr, find and assign the first local adapter
    if(nullptr==client) {
       QList<QBluetoothHostInfo> localAdapters = QBluetoothLocalDevice::allDevices();
       if(localAdapters.empty()) {
          qDebug() << "failed to find a local adapter";
          close();
       }
       else {
         client = new QBluetoothLocalDevice(localAdapters.at(0).address());
         qDebug() << "added local adapter from list";
       }
    }

    agent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &MainWindow::deviceDiscovered);
    connect(agent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error),
            this, &MainWindow::deviceScanError);
    connect(agent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &MainWindow::deviceDiscoveryComplete);
    agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

    connect(ui->saveButton,&QPushButton::clicked, this, &MainWindow::writeMeasurement);

}

MainWindow::~MainWindow()
{
    delete ui;
    if(nullptr!=client) {
      delete client;
    }
    if(nullptr!=peripheral) {
      delete peripheral;
    }
    if(nullptr!=agent) {
      delete agent;
    }
    if(nullptr!=controller) {
      delete controller;
    }
}

void MainWindow::readSettings()
{
   QSettings settings(this->appDir.filePath("bt.ini"),QSettings::IniFormat);
   QString address = settings.value("client/address").toString();
   if(!address.isEmpty()) {
     client = new QBluetoothLocalDevice(QBluetoothAddress(address));

     if(!client->isValid()) {
         qDebug() << "client is invalid";
         close();
     }
     client->setHostMode(settings.value("client/hostmode").value<QBluetoothLocalDevice::HostMode>());
     qDebug() << "constructed client from settings file";

     //QBluetoothLocalDevice::HostMode mode = client->hostMode();

     if(client->hostMode()==QBluetoothLocalDevice::HostPoweredOff)
     {
         qDebug() << "client is powered off";
         client->powerOn();
     }
     if(client->hostMode()!=QBluetoothLocalDevice::HostDiscoverable)
     {
         qDebug() << "setting client host mode to host discoverable";
         client->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
     }

     /*
     else if (mode==QBluetoothLocalDevice::HostConnectable)
         qDebug() << "client is host connectable";
     else if (mode==QBluetoothLocalDevice::HostDiscoverable)
         qDebug() << "client is host discoverable";
     else if (mode==QBluetoothLocalDevice::HostDiscoverableLimitedInquiry)
         qDebug() << "client is host discoverable limited inquiry";
     else
         qDebug() << "unknown client host mode";
     */
   }

   address = settings.value("peripheral/address").toString();
   if(!address.isEmpty()) {
     qDebug() << "constructed peripheral from settings file";
   }

}

void MainWindow::writeSettings()
{
   QSettings settings(this->appDir.filePath("bt.ini"),QSettings::IniFormat);
   if(nullptr!=client) {
     settings.setValue("client/name",client->name());
     settings.setValue("client/address",client->address().toString());
     settings.setValue("client/hostmode",client->hostMode());
     qDebug() << "wrote client to settings file";
   }
   if(nullptr!=peripheral) {
     settings.setValue("peripheral/name",peripheral->name());
     settings.setValue("peripheral/address",peripheral->address().toString());
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

    // we can stop the scanning once we find our target device
    if(info.address().toString()==peripheralMAC) {
        qDebug() << "Found target peripheal with MAC " << info.address().toString() << " ... stopping scan";
        agent->stop();
        peripheral = new QBluetoothDeviceInfo(info);
        controller = QLowEnergyController::createCentral(*peripheral);

        connect(controller, &QLowEnergyController::connected,
                this,&MainWindow::discoverServices);

        connect(controller, &QLowEnergyController::disconnected,
                this,&MainWindow::deviceDisconnected);

        connect(controller, &QLowEnergyController::serviceDiscovered,
                this, &MainWindow::serviceDiscovered);

        connect(controller, QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error),
                this, &MainWindow::serviceScanError);

        connect(controller, &QLowEnergyController::discoveryFinished,
                this, &MainWindow::serviceDiscoveryComplete);

        controller->setRemoteAddressType(QLowEnergyController::PublicAddress);

        ui->connectButton->setEnabled(true);

        connect(ui->connectButton,&QPushButton::clicked,
            this,[this](){
            qDebug() << "controller connecting to device";
            controller->connectToDevice();
            });
    }
}

void MainWindow::deviceDiscoveryComplete()
{
    // list all the devices
    QList<QBluetoothDeviceInfo> devices = agent->discoveredDevices();
    qDebug() << "Found " << QString::number(devices.count()) << " devices";
}

void MainWindow::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        qDebug() << "The Bluetooth adaptor is powered off, power it on before doing discovery.";
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        qDebug() << "Writing or reading from the device resulted in an error.";
    else {
        static QMetaEnum qme = agent->metaObject()->enumerator(
                    agent->metaObject()->indexOfEnumerator("Error"));
        qDebug() << "Error: " << QLatin1String(qme.valueToKey(error));
    }
}

void MainWindow::deviceDisconnected()
{
    ui->connectButton->setEnabled(true);
    qDebug() << (nullptr==controller ? "controller not activated error" : "controller disconnected from peripheral");
}

void MainWindow::discoverServices()
{
    ui->connectButton->setEnabled(false); // we are connected, so dont allow repeated connect button clicks until we disconnect
    qDebug() << (controller->remoteAddressType()==QLowEnergyController::RandomAddress ? "remote address type" : "public address type");
    qDebug() << "controller finding device services";
    controller->discoverServices();
}

void MainWindow::serviceDiscovered(const QBluetoothUuid &serviceUuid)
{
  qDebug() << "service discovered ";
  if(serviceUuid == QBluetoothUuid(QBluetoothUuid::HealthThermometer))
  {
      qDebug() << "discovered the health thermometer service";
      foundThermometer = true;
  }
}

void MainWindow::serviceDiscoveryComplete()
{
  qDebug() << "controller service discovery complete";

  //
  if(!foundThermometer)
  {
      qDebug() << "error: did not discover the health thermometer service";
      return;
  }

  QLowEnergyService *service = controller->createServiceObject(QBluetoothUuid(QBluetoothUuid::HealthThermometer));
  if (!service) {
      qDebug() << "Cannot create service for thermometer";
      return;
  }

  connect(service, &QLowEnergyService::stateChanged, this, &MainWindow::serviceDetailsState);

  connect(service, &QLowEnergyService::characteristicChanged, this, &MainWindow::updateTemperatureValue);

  connect(service, &QLowEnergyService::descriptorWritten, this, &MainWindow::confirmedDescriptorWrite);

  service->discoverDetails();
}

void MainWindow::confirmedDescriptorWrite(const QLowEnergyDescriptor& d, const QByteArray& a)
{
   qDebug() << "confirmed descriptor write with value: " << BLEInfo::valueToString(a);

   if(d.isValid() && a == QByteArray::fromHex("0100"))
   {
      qDebug() << "success write";
      //controller->disconnectFromDevice();
   }
   else
   {
       qDebug() << "write error";
   }
}

void MainWindow::serviceDetailsState(QLowEnergyService::ServiceState newState)
{
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
    const QLowEnergyCharacteristic tempChar = service->characteristic(QBluetoothUuid(QBluetoothUuid::TemperatureMeasurement));
    if (!tempChar.isValid())
    {
        qDebug() << "Temperature measurement data not found.";
        return;
    }

    QLowEnergyDescriptor desc = tempChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
    if (desc.isValid())
    {
        qDebug() << "LE descriptor found ... writing";
        service->writeDescriptor(desc, QByteArray::fromHex("0100"));
    }
/*
    const QList<QLowEnergyCharacteristic> chars = service->characteristics();
    for (const QLowEnergyCharacteristic &ch : chars) {
        qDebug() << "characteristic: " << ch.name() << (ch.isValid()? " valid ":" invalid ") << ", uuid: "<< BLEInfo::uuidToString(ch.uuid())<< " handle: " << BLEInfo::handleToString(ch.handle());
        qDebug() << "permissions: " << BLEInfo::permissionToString(ch);
        qDebug() << "contains " << QString::number(ch.descriptors().count()) << " descriptors";
        qDebug() << "value: " << BLEInfo::valueToString(ch.value()) << " " << QString::number(ch.value().size()) << " bytes";

        if(ch.descriptors().count()>0)
        {
          QList<QLowEnergyDescriptor> descs = ch.descriptors();
          QLowEnergyDescriptor des = descs.at(0);
          qDebug() << "descriptor : " <<  des.name() << (des.isValid()? " valid ":" invalid ") << ", uuid: " << BLEInfo::uuidToString(des.uuid()) << " handle: " << BLEInfo::handleToString(des.handle());
          qDebug() << "value: " << BLEInfo::valueToString(des.value()) << " " << QString::number(des.value().size()) << " bytes";
        }

        // NOTE that desc.name() and QBluetoothUuid::descriptorToString(des.type()) produce the same result
        //
    }
*/


    /*

    switch (newState) {
    case QLowEnergyService::DiscoveringServices:
        qDebug() << "Discovering services...";
        break;
    case QLowEnergyService::ServiceDiscovered:
    {
        setInfo(tr("Service discovered."));

        const QLowEnergyCharacteristic hrChar = m_service->characteristic(QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement));
        if (!hrChar.isValid()) {
            setError("HR Data not found.");
            break;
        }

        m_notificationDesc = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (m_notificationDesc.isValid())
            m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));

        break;
    }
    default:
        //nothing for now
        break;
    }
    */
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
   measurement.clear();
   measurement.insert("Temperature", QVariant(t_value));
   measurement.insert("Format", QVariant(t_format_str));
   measurement.insert("Type", QVariant(t_type_str));
   measurement.insert("DateTime",QVariant(QDateTime(QDate(y_value,m_value,d_value),QTime(h_value,j_value,s_value))));

   ui->saveButton->setEnabled(true);
}

void MainWindow::serviceScanError(QLowEnergyController::Error error)
{
     qDebug() << "controller error string: " << controller->errorString();

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
 //   else if (error == QLowEnergyController::AuthorizationError)
 //       qDebug() << "The local Bluetooth device closed the connection due to insufficient authorization.";

    else {
        static QMetaEnum qme = controller->metaObject()->enumerator(
                    agent->metaObject()->indexOfEnumerator("Error"));
        qDebug() << "Error: " << QLatin1String(qme.valueToKey(error));
    }
}

void MainWindow::writeMeasurement()
{
   qDebug() << "begin write process ... ";

   QJsonObject json;
   QMap<QString,QVariant>::const_iterator it = measurement.constBegin();
   while(it != measurement.constEnd()) {
     json.insert(it.key(),QJsonValue::fromVariant(it.value()));
     ++it;
   }
   // insert pariticpant barcode
   QString barcode = ui->iDlineEdit->text().simplified().remove(" ");
   json.insert("Barcode",QJsonValue(barcode));
   // the output will be of the form <participant ID>_<now>_<devicename>.json
   QStringList jsonFile;
   jsonFile << barcode;
   jsonFile << QDate().currentDate().toString("yyyyMMdd");
   jsonFile << "bt_masimo.json";
   QFile saveFile( this->appDir.filePath( jsonFile.join("_") ) );
   saveFile.open(QIODevice::WriteOnly);
   saveFile.write(QJsonDocument(json).toJson());

   qDebug() << "wrote to file " << this->appDir.filePath( jsonFile.join("_") );
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
