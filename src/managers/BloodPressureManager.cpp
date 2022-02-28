#include "BloodPressureManager.h"

#include "BPMCommunication.h"

#include <QDebug>
#include <QJsonObject>
#include <QSettings>
#include <QStandardItemModel>
#include <QCoreApplication>
#include <QUsb>

BloodPressureManager::BloodPressureManager(QObject* parent) 
    : ManagerBase(parent)
    , m_comm(new BPMCommunication(this))
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

    connect(this, &BloodPressureManager::attemptConnection, m_comm, &BPMCommunication::connectToBpm);
    connect(this, &BloodPressureManager::startMeasurement, m_comm, &BPMCommunication::measure);
    connect(this, &BloodPressureManager::abortMeasurement, m_comm, &BPMCommunication::abort);

    // Connect comm signals with slots
    connect(m_comm, &BPMCommunication::abortFinished, this, &BloodPressureManager::abortComplete);
    connect(m_comm, &BPMCommunication::connectionStatus, this, &BloodPressureManager::connectionStatusChanged);
    connect(m_comm, &BPMCommunication::measurementReady, this, &BloodPressureManager::measurementAvailable);
    connect(m_comm, &BPMCommunication::averageReady, this, &BloodPressureManager::averageAvailable);
    connect(m_comm, &BPMCommunication::finalReviewReady, this, &BloodPressureManager::finalReviewAvailable);
    connect(m_comm, &BPMCommunication::measurementError, this, &BloodPressureManager::message);

    emit dataChanged();
}

void BloodPressureManager::loadSettings(const QSettings& settings)
{
    QString pid = settings.value(getGroup() + "/client/pid").toString();
    selectDevice(pid);
}

void BloodPressureManager::saveSettings(QSettings* settings) const
{
    int pid = getPid();
    if(0 != pid)
    {
        settings->beginGroup(getGroup());
        settings->setValue("client/pid", pid);
        settings->endGroup();
        if(m_verbose)
            qDebug() << "wrote pid to settings file";
    }
}

void BloodPressureManager::selectDevice(const QString& pid)
{
    m_pid = pid.toInt();
    if(armInformationSet() && connectionInfoSet())
    {
        emit message(tr("Ready to connect..."));
        emit canConnectDevice();
    }
}

QList<int> BloodPressureManager::findAllPids() const
{
    QUsb::IdList usbDeviceList = QUsb::devices();

    QList<int> matchingPids;
    foreach(auto entry, usbDeviceList)
    {
        qDebug() << entry;
        if(m_vid == entry.vid)
        {
            int pid = entry.pid;
            qDebug() << "found one with pid = " << pid;
            if(!matchingPids.contains(pid))
            {
                matchingPids.append(pid);
            }
        }
    }
    return matchingPids;
}


bool BloodPressureManager::connectionInfoSet() const
{
    if(CypressConstants::RunMode::Simulate == m_mode)
    {
        return true;
    }
    return (0 < m_pid);
}

void BloodPressureManager::setCuffSize(const QString &size)
{
  m_test.setCuffSize(size);
  if(armInformationSet() && connectionInfoSet())
  {
      emit message(tr("Ready to connect..."));
      emit canConnectDevice();
  }
}

void BloodPressureManager::setSide(const QString &side)
{
  m_test.setSide(side);
  if(armInformationSet() && connectionInfoSet())
  {
      emit message(tr("Ready to connect..."));
      emit canConnectDevice();
  }
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

    if(CypressConstants::RunMode::Simulate == m_mode)
    {
      //TODO: generate simulated readings and emit canWrite signal
      //
      return;
    }

    clearData();
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
    if(CypressConstants::RunMode::Simulate == m_mode)
    {
      return;
    }
    emit abortMeasurement(QThread::currentThread());
    while(!m_aborted)
    {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

void BloodPressureManager::finish()
{
    disconnectDevice();
    m_deviceData.reset();
    m_test.reset();
}

void BloodPressureManager::measurementAvailable(
  const int& sbp, const int& dbp, const int& pulse,
  const QDateTime& start, const QDateTime& end, const int& readingNum)
{
    m_test.addMeasurement(sbp, dbp, pulse, start, end, readingNum);
    emit dataChanged();
}

void BloodPressureManager::averageAvailable(
  const int& sbp, const int& dbp, const int& pulse)
{
    m_test.addAverageMeasurement(sbp, dbp, pulse);
    emit dataChanged();
}

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
        emit canMeasure();
        return;
    }
}

void BloodPressureManager::connectDevice()
{
    // Do not attempt connection unless the required
    // connection info has not been set
    if(!connectionInfoSet())
    {
      if(m_verbose)
        qDebug() << "Connection info has not been set";
      return;
    }

    if(CypressConstants::RunMode::Simulate == m_mode)
    {
      emit message(tr("Ready to measure..."));
      emit canMeasure();
      return;
    }

    // Move comm to comm thread and start comm thread
    if(m_comm->thread() != &m_thread)
    {
        m_comm->moveToThread(&m_thread);
        m_thread.start();
    }

    // Attempt to connect to bpm
    emit attemptConnection(m_vid, m_pid);
}

void BloodPressureManager::connectionStatusChanged(const bool& connected)
{
    if(connected)
    {
        // signal the GUI that the measure button can be clicked
        //
        emit message(tr("Ready to measure..."));
        emit canMeasure();
    }
    else
    {
        //emit connectionFailure();
        // TODO: Show the following message to user 
        //       "There was a problem connecting to the Blood Pressure Monitor.
        //        Please ensure that the bpm is plugged in, turned on and connected 
        //        to the computer"
        emit message(tr("Failed to connect..."));
    }
}

void BloodPressureManager::deviceInfoAvailable()
{
    QString product = m_comm->product();
    QString serial = m_comm->serialNumber();
    QString manufacturer = m_comm->manufacturer();
    QString version = m_comm->version();
    if(!product.isEmpty() && 0 < product.length())
      m_deviceData.setCharacteristic("device_product",product);
    if(!serial.isEmpty() && 0 < serial.length())
      m_deviceData.setCharacteristic("device_serial_number",serial);
    if(!manufacturer.isEmpty() && 0 < manufacturer.length())
      m_deviceData.setCharacteristic("device_manufacturer",manufacturer);
    if(!version.isEmpty() && 0 < version.length())
      m_deviceData.setCharacteristic("device_version",version);
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
