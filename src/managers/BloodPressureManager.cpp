#include "BloodPressureManager.h"

#include "BPMCommunication.h"

#include <QCoreApplication>
#include <QDebug>
#include <QHidDevice>
#include <QJsonObject>
#include <QSettings>
#include <QStandardItemModel>
#include <QUsb>

BloodPressureManager::BloodPressureManager(QObject* parent) 
    : ManagerBase(parent)
    , m_comm(new BPMCommunication())
{
    setGroup("bloodpressure");
    m_col = 1;
    m_row = 8;

    m_inputKeyList << "barcode";
    m_inputKeyList << "language";

    if(m_verbose)
      qDebug() << "Manager created on thread: " << QThread::currentThreadId();
}

BloodPressureManager::~BloodPressureManager()
{
  delete m_comm;
}

void BloodPressureManager::start()
{
    // connect manager to communication
    connect(this, &BloodPressureManager::attemptConnection, m_comm, &BPMCommunication::connect);
    connect(this, &BloodPressureManager::startMeasurement, m_comm, &BPMCommunication::measure);
    connect(this, &BloodPressureManager::abortMeasurement, m_comm, &BPMCommunication::abort);

    // connect communication signals with slots
    connect(m_comm, &BPMCommunication::abortFinished, this, &BloodPressureManager::abortComplete);
    connect(m_comm, &BPMCommunication::connectionStatus, this, &BloodPressureManager::connectionStatusChanged);
    connect(m_comm, &BPMCommunication::measurementReady, this, &BloodPressureManager::measurementAvailable);
    connect(m_comm, &BPMCommunication::averageReady, this, &BloodPressureManager::averageAvailable);
    connect(m_comm, &BPMCommunication::finalReviewReady, this, &BloodPressureManager::finalReviewAvailable);
    connect(m_comm, &BPMCommunication::measurementError, this, &BloodPressureManager::message);

    // move communication to thread and start
    if(m_comm->thread() != &m_thread)
    {
      m_comm->moveToThread(&m_thread);
      m_thread.start();
    }

    scanDevices();
    emit dataChanged();
}

bool BloodPressureManager::isDefined(const QString &label) const
{
    if(CypressConstants::RunMode::Simulate == m_mode)
    {
      return true;
    }
    bool defined = false;
    if(m_deviceList.contains(label))
    {
      QUsb::Id info = m_deviceList.value(label);
      defined = !(0 == info.pid || 0 == info.vid);
    }
    return defined;
}

void BloodPressureManager::loadSettings(const QSettings& settings)
{
    QUsb::Id info;
    info.pid = settings.value(getGroup() + "/client/pid").toInt();
    info.vid = settings.value(getGroup() + "/client/vid").toInt();
    selectDevice(info);
}

void BloodPressureManager::saveSettings(QSettings* settings) const
{
    if(isDefined(m_deviceName))
    {
        settings->beginGroup(getGroup());
        settings->setValue("client/pid", m_device.pid);
        settings->setValue("client/pid", m_device.vid);
        settings->endGroup();
        if(m_verbose)
            qDebug() << "wrote pid/vid to settings file";
    }
}

void BloodPressureManager::selectDeviceById(const QUsb::Id& info)
{
    QMap<QString,QUsb::Id>::const_iterator it = m_deviceList.constBegin();
    bool found = false;
    QString label;
    while(it != m_deviceList.constEnd() && !found)
    {
      QUsb::Id value = it.value();
      found = info.pid == value.pid && info.vid == value.pid;
      if(found) label = it.key();
      ++it;
    }
    if(found)
      selectDevice(label);
}

void BloodPressureManager::selectDevice(const QString &label)
{
    if(m_deviceList.contains(label))
    {
      QUsb::Id info = m_deviceList.value(label);
      setProperty("deviceName",label);
      setDevice(info);
      if(m_verbose)
         qDebug() << "device selected from label " <<  label;
    }
}

void BloodPressureManager::scanDevices()
{
    m_deviceList.clear();
    emit message(tr("Discovering devices..."));
    emit scanningDevices();
    if(m_verbose)
      qDebug() << "start scanning for devices...";
    if(CypressConstants::RunMode::Simulate == m_mode)
    {
      QUsb::Id info;
      info.pid = 1;
      info.vid = BPTRU_VENDOR_ID;
      QString label = m_deviceName.isEmpty() ? "simulated_device" : m_deviceName;
      m_deviceList.insert(label,info);
      emit deviceDiscovered(label);
      if(m_verbose)
        qDebug() << "found HID device " << label;
      setDevice(info);
      return;
    }
    QHidDevice hid_device;
    foreach(auto info, QUsb::devices())
    {
      // try to open the device as an hid
      hid_device.open(info.vid,info.pid);
      if(hid_device.isOpen())
      {
          QString serial = hid_device.serialNumber();
          QString manufacturer = hid_device.manufacturer();
          QString product = hid_device.product();
          QString label = QString("%1_%2_%3").arg(manufacturer,product,serial);
          if(!m_deviceList.contains(label))
          {
              m_deviceList.insert(label,info);
              emit deviceDiscovered(label);
          }
          if(m_verbose)
          {
              qDebug() << "product id:" << QString::number(info.pid);
              qDebug() << "vendor id:" << QString::number(info.vid);
              qDebug() << "manufacturer:" << manufacturer;
              qDebug() << "product:" << product;
              qDebug() << "serial number:" << serial;
          }
          hid_device.close();
          continue;
      }
    }
    if(m_verbose)
        qDebug() << "Found " << QString::number(m_deviceList.count()) << " HID devices";

    if(0 == m_deviceList.count())
    {
        qDebug() << "no devices found";
        return;
    }
    // if we have a pid and vid from the ini file, check if it is still available on the system
    //
    bool found = false;
    QUsb::Id info;
    QString label;
    if(!m_deviceName.isEmpty())
    {
        QMap<QString,QUsb::Id>::const_iterator it = m_deviceList.constBegin();
        while(it != m_deviceList.constEnd() && !found)
        {
          label = it.key();
          found = label == m_deviceName;
          if(found) info = it.value();
          ++it;
        }
    }
    if(found)
    {
      if(m_verbose)
        qDebug() << "found the ini stored HID device  " << m_deviceName;

      emit deviceSelected(label);
      setDevice(info);
    }
    else
    {
      // select a device from the list of scanned devices
      //
      emit message(tr("Ready to select..."));
      emit canSelectDevice();
    }
}

void BloodPressureManager::setDevice(const QUsb::Id &info)
{
    m_deviceData.reset();
    m_device = QUsb::Id();

    if(CypressConstants::RunMode::Simulate == m_mode)
    {
       // get the device data
       m_deviceData.setCharacteristic("usb_hid_product_id", info.pid);
       m_deviceData.setCharacteristic("usb_hid_vendor_id", info.vid);
       m_deviceData.setCharacteristic("usb_hid_manufacturer", "BpTru");
       m_deviceData.setCharacteristic("usb_hid_serial_number", "simulated");
       m_deviceData.setCharacteristic("usb_hid_product", "BPM200");
       emit message(tr("Ready to connect..."));
       emit canConnectDevice();
       return;
    }

    if(m_deviceName.isEmpty() || 0 == info.pid || 0 == info.vid)
    {
        if(m_verbose)
            qDebug() << "ERROR: no device available";
        return;
    }
    if(m_verbose)
        qDebug() << "ready to connect to device " << m_deviceName;

    QHidDevice device;
    if(device.open(info.pid,info.vid))
    {
        // signal the GUI that the device is connectable so that
        // the connect button can be clicked
        //
        QString serial = device.serialNumber();
        QString manufacturer = device.manufacturer();
        QString product = device.product();
        m_deviceData.setCharacteristic("usb_hid_product_id", info.pid);
        m_deviceData.setCharacteristic("usb_hid_vendor_id", info.vid);
        m_deviceData.setCharacteristic("usb_hid_manufacturer", manufacturer);
        m_deviceData.setCharacteristic("usb_hid_serial_number", serial);
        m_deviceData.setCharacteristic("usb_hid_product", product);
        m_device = info;

        emit message(tr("Ready to connect..."));
        emit canConnectDevice();
        device.close();
    }
}

void BloodPressureManager::setCuffSize(const QString &size)
{
  m_test.setCuffSize(size);
}

void BloodPressureManager::setSide(const QString &side)
{
  m_test.setSide(side);
}

QJsonObject BloodPressureManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    json.insert("device",m_deviceData.toJsonObject());
    return json;
}

void BloodPressureManager::buildModel(QStandardItemModel* model) const
{
    // add measurements one row of one columns at a time
    //
    int n_total = m_test.getNumberOfMeasurements();
    int n_row = qMax(1, n_total);
    if(model->rowCount() != n_row)
    {
        model->setRowCount(n_row);
    }
    
    // Add first measurement
    int row = 0;
    QString firstMeasurement = m_test.firstMeasurementToString();
    if(!firstMeasurement.isEmpty() && 0 < firstMeasurement.length())
    {
        QStandardItem* firstItem = model->item(row);
        if(Q_NULLPTR == firstItem)
        {
            firstItem = new QStandardItem();
            model->setItem(row, 0, firstItem);
        }
        firstItem->setData(firstMeasurement, Qt::DisplayRole);
        row++;
    }    

    for(int i = 0; i < n_total; i++)
    {
        BloodPressureMeasurement measurement = m_test.getMeasurement(i);
        QString measurementStr = measurement.isValid() ? measurement.toString() : "NA";
        QStandardItem* item = model->item(row);
        if(Q_NULLPTR == item)
        {
            item = new QStandardItem();
            model->setItem(row, 0, item);
        }
        item->setData(measurementStr, Qt::DisplayRole);
        row++;
    }

    // Add avg measurement
    QString avgMeasurement = m_test.avgMeasurementToString();
    if(!avgMeasurement.isEmpty() && 0 < avgMeasurement.length())
    {
        QStandardItem* avgItem = model->item(row);
        if(Q_NULLPTR == avgItem)
        {
            avgItem = new QStandardItem();
            model->setItem(row, 0, avgItem);
        }
        avgItem->setData(avgMeasurement, Qt::DisplayRole);
        row++;
    }

    // Add all avg measurement
    QString allAvgMeasurement = m_test.allAvgMeasurementToString();
    if(!allAvgMeasurement.isEmpty() && 0 < allAvgMeasurement.length())
    {
        QStandardItem* allAvgItem = model->item(row);
        if(Q_NULLPTR == allAvgItem)
        {
            allAvgItem = new QStandardItem();
            model->setItem(row, 0, allAvgItem);
        }
        allAvgItem->setData(allAvgMeasurement, Qt::DisplayRole);
    }
}

void BloodPressureManager::measure()
{
    if(m_verbose)
      qDebug() << "starting process from measure";

    emit message(tr("Measuring blood pressure..."));
    clearData();
    if(CypressConstants::RunMode::Simulate == m_mode)
    {
      //TODO: generate simulated readings and emit canWrite signal
      //
      return;
    }

    emit startMeasurement();
}

void BloodPressureManager::setInputData(const QMap<QString, QVariant>& input)
{
    if (CypressConstants::RunMode::Simulate == m_mode)
    {
        m_inputData["barcode"] = "00000000";
        m_inputData["language"] = "english";
        return;
    }
    bool ok = true;
    for(auto&& x : m_inputKeyList)
    {
        if(!input.contains(x))
        {
            ok = false;
            break;
        }
        else
            m_inputData[x] = input[x];
    }
    if(!ok)
        m_inputData.clear();
}

void BloodPressureManager::clearData()
{
    m_test.reset();
    emit dataChanged();
}

void BloodPressureManager::disconnectDevice()
{
    m_aborted = false;
    if(CypressConstants::RunMode::Simulate == m_mode)
    {
      emit message(tr("Ready to connect..."));
      emit canConnectDevice();
      return;
    }

    emit abortMeasurement(QThread::currentThread());
    while(!m_aborted)
    {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    m_comm->disconnect();
    emit message(tr("Ready to connect..."));
    emit canConnectDevice();
}

void BloodPressureManager::finish()
{
    disconnectDevice();
    m_thread.quit();
    m_deviceData.reset();
    m_test.reset();
}

// slot for BPMCommunication
//
void BloodPressureManager::measurementAvailable(
  const int& sbp, const int& dbp, const int& pulse,
  const QDateTime& start, const QDateTime& end, const int& readingNum)
{
    m_test.addMeasurement(sbp, dbp, pulse, start, end, readingNum);
    emit dataChanged();
}

// slot for BPMCommunication
//
void BloodPressureManager::averageAvailable(
  const int& sbp, const int& dbp, const int& pulse)
{
    m_test.addAverageMeasurement(sbp, dbp, pulse);
    emit dataChanged();
}

// slot for BPMCommunication
//
void BloodPressureManager::finalReviewAvailable(
  const int& sbp, const int& dbp, const int& pulse)
{
    if(m_test.verifyReviewData(sbp, dbp, pulse))
    {
        // emit the can write signal
        emit message(tr("Ready to save results..."));
        emit canWrite();
    }
    else
    {
        // TODO: error message on failure?
        if(m_test.armInformationSet())
          emit canMeasure();
    }
}

void BloodPressureManager::connectDevice()
{
    if(CypressConstants::RunMode::Simulate == m_mode)
    {
        if(m_test.armInformationSet())
        {
          emit message(tr("Ready to measure..."));
          emit canMeasure();
        }
        else
        {
          emit message(tr("Arm information not set..."));
        }
        return;
    }

    emit attemptConnection(m_device);
}

// slot for BPMCommunication
//
void BloodPressureManager::connectionStatusChanged(const bool& connected)
{
    if(connected)
    {
        // signal the GUI that the measure button can be clicked
        //
        if(m_test.armInformationSet())
        {
          emit message(tr("Ready to measure..."));
          emit canMeasure();
        }
        else
        {
          emit message(tr("Arm information not set..."));
        }
    }
    else
    {
        emit message(tr("Ready to connect..."));
        emit canConnectDevice();
    }
}

// slot for BPMCommunication
//
void BloodPressureManager::deviceInfoAvailable()
{
    QString product = m_comm->product();
    QString serial = m_comm->serialNumber();
    QString manufacturer = m_comm->manufacturer();
    QString version = m_comm->version();
    if(!product.isEmpty() && 0 < product.length())
      m_deviceData.setCharacteristic("usb_hid__product",product);
    if(!serial.isEmpty() && 0 < serial.length())
      m_deviceData.setCharacteristic("usb_hid_serial_number",serial);
    if(!manufacturer.isEmpty() && 0 < manufacturer.length())
      m_deviceData.setCharacteristic("usb_hid_manufacturer",manufacturer);
    if(!version.isEmpty() && 0 < version.length())
      m_deviceData.setCharacteristic("usb_hid_version",version);
}

void BloodPressureManager::abortComplete(const bool& success)
{
    Q_UNUSED(success)
    //TODO: remove debug or refactor into manager class
    //
    if(m_verbose)
      qDebug() << "BPM200: Abort complete";

    m_thread.quit();
    m_thread.wait();
    m_aborted = true;
}
