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
    setGroup("blood_pressure");

    // test number, start time, end time, systolic, diastolic, pulse
    //
    m_col = 6;

    // 6 actual measurements
    // the average of the last 5, the average of all 6 not displayed
    //
    m_row = 6;

    m_inputKeyList << "barcode";
    m_inputKeyList << "language";

    if(m_verbose)
      qDebug() << "Manager created on thread: " << QThread::currentThreadId();

    m_test.setMaximumNumberOfMeasurements(m_row);
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

    // connect communication to manager
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
    if(Constants::RunMode::modeSimulate == m_mode)
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
    selectDeviceById(info);
}

void BloodPressureManager::saveSettings(QSettings* settings) const
{
    if(isDefined(m_deviceName))
    {
        settings->beginGroup(getGroup());
        settings->setValue("client/pid", m_device.pid);
        settings->setValue("client/vid", m_device.vid);
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
    if(Constants::RunMode::modeSimulate == m_mode)
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
      if(0 != m_vendorIDFilter && m_vendorIDFilter != info.vid)
          continue;

      // try to open the usb hid device
      //
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
    qDebug() << "manager setting device with info";
    m_deviceData.reset();
    m_device = QUsb::Id();

    if(Constants::RunMode::modeSimulate == m_mode)
    {
       // get the device data
       m_deviceData.setAttribute("usb_hid_product_id", info.pid);
       m_deviceData.setAttribute("usb_hid_vendor_id", info.vid);
       m_deviceData.setAttribute("usb_hid_manufacturer", "BpTru");
       m_deviceData.setAttribute("usb_hid_serial_number", "simulated");
       m_deviceData.setAttribute("usb_hid_product", "BPM200");
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
        QString serial = device.serialNumber();
        QString manufacturer = device.manufacturer();
        QString product = device.product();
        m_deviceData.setAttribute("usb_hid_product_id", info.pid);
        m_deviceData.setAttribute("usb_hid_vendor_id", info.vid);
        m_deviceData.setAttribute("usb_hid_manufacturer", manufacturer);
        m_deviceData.setAttribute("usb_hid_serial_number", serial);
        m_deviceData.setAttribute("usb_hid_product", product);
        m_device = info;

        // signal the GUI that the device is connectable so that
        // the connect button can be clicked
        //
        emit message(tr("Ready to connect..."));
        emit canConnectDevice();
        device.close();
    }
}

void BloodPressureManager::setCuffSize(const QString &size)
{
  if(size.isNull() || 0 == size.length()) return;
  if(size.toLower() != m_cuffSize)
  {
    m_cuffSize = size.toLower();
    m_test.setCuffSize(m_cuffSize);
    emit cuffSizeChanged(m_cuffSize);
  }
}

void BloodPressureManager::setSide(const QString &side)
{
  if(side.isNull() || 0 == side.length()) return;
  if(side.toLower() != m_side)
  {
    m_side = side.toLower();
    m_test.setSide(m_side);
    emit sideChanged(m_side);
  }
}

QJsonObject BloodPressureManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    json.insert("device",m_deviceData.toJsonObject());
    json.insert("test_input",m_inputData);
    return json;
}

void BloodPressureManager::buildModel(QStandardItemModel* model) const
{
    // add measurements one row of one columns at a time
    //
    int n_total = m_test.getNumberOfMeasurements();
    for(int row = 0; row < n_total; row++)
    {
        BloodPressureMeasurement m = m_test.getMeasurement(row);
        if(m_verbose)
          qDebug() <<"model build" << m;

        QStringList list = (QStringList()
          << m.getAttribute("reading_number").toString()
          << m.getAttribute("start_time").toString()
          << m.getAttribute("end_time").toString()
          << m.getAttribute("systolic").toString()
          << m.getAttribute("diastolic").toString()
          << m.getAttribute("pulse").toString());
        for(int col = 0; col < m_col; col ++)
        {
            QStandardItem* item = model->item(row,col);
            if(Q_NULLPTR == item)
            {
                item = new QStandardItem();
                model->setItem(row, col, item);
            }
            item->setData(list.at(col), Qt::DisplayRole);
        }
    }
}

// slot for UI communication
//
void BloodPressureManager::measure()
{
    if(m_verbose)
      qDebug() << "starting process from measure";

    emit message(tr("Measuring blood pressure..."));
    clearData();
    if(Constants::RunMode::modeSimulate == m_mode)
    {
      m_test.simulate();
      emit dataChanged();
      if(m_test.isValid())
      {
        emit message(tr("Ready to save results..."));
        emit canWrite();
      }
      return;
    }

    emit startMeasurement();
}

void BloodPressureManager::setInputData(const QJsonObject& input)
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
    foreach(auto key, m_inputKeyList)
    {
      if(!m_inputData.contains(key))
      {
        ok = false;
        if(m_verbose)
          qDebug() << "ERROR: missing expected input " << key;
        break;
      }
      const QVariant value = m_inputData[key].toVariant();
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
      m_inputData = QJsonObject();
    }
    else
    {
      // optional json inputs
      //
      if(m_inputData.contains("side"))
          setSide(m_inputData["side"].toString());
      if(m_inputData.contains("cuff_size"))
          setCuffSize(m_inputData["cuff_size"].toString());
    }
}

void BloodPressureManager::clearData()
{
    m_test.reset();
    emit dataChanged();
}

// slot for UI communication
//
void BloodPressureManager::disconnectDevice()
{
    m_aborted = false;
    if(Constants::RunMode::modeSimulate == m_mode)
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

// slot for UI communication
//
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
  const int& readingNum, const int& sbp, const int& dbp, const int& pulse,
  const QDateTime& start, const QDateTime& end)
{
    BloodPressureMeasurement m(readingNum, sbp, dbp, pulse, start, end);
    if(m.isValid())
    {
      m_test.addMeasurement(m);
    }
    emit dataChanged();
}

// slot for BPMCommunication
//
void BloodPressureManager::averageAvailable(
  const int& sbp, const int& dbp, const int& pulse)
{
    m_test.addDeviceAverage(sbp, dbp, pulse);
    emit dataChanged();
}

// slot for BPMCommunication
//
void BloodPressureManager::finalReviewAvailable(
  const int& sbp, const int& dbp, const int& pulse)
{
    if(m_test.verifyDeviceAverage(sbp, dbp, pulse))
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

// slot for UI communication
//
void BloodPressureManager::connectDevice()
{
    if(Constants::RunMode::modeSimulate == m_mode)
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
      m_deviceData.setAttribute("usb_hid_product",product);
    if(!serial.isEmpty() && 0 < serial.length())
      m_deviceData.setAttribute("usb_hid_serial_number",serial);
    if(!manufacturer.isEmpty() && 0 < manufacturer.length())
      m_deviceData.setAttribute("usb_hid_manufacturer",manufacturer);
    if(!version.isEmpty() && 0 < version.length())
      m_deviceData.setAttribute("usb_hid_version",version);
}

// slot for BPMCommunication
//
void BloodPressureManager::abortComplete(const bool& success)
{
    Q_UNUSED(success)

    if(m_verbose)
      qDebug() << "BPM200: Abort complete";

    m_thread.quit();
    m_thread.wait();
    m_aborted = true;
}
