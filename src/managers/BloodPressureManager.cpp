#include "BloodPressureManager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>
#include <QStandardItemModel>

BloodPressureManager::BloodPressureManager(QObject* parent) 
    : ManagerBase(parent), m_bpm(new BPM200())
{
    setGroup("bloodpressure");
    m_col = 1;
    m_row = 8;

    m_inputKeyList << "barcode";
    m_inputKeyList << "language";

    if(m_verbose)
      qDebug() << "Manager created on thread: " << QThread::currentThreadId();
}

void BloodPressureManager::start()
{
    setupConnections();
    emit dataChanged();
}

void BloodPressureManager::loadSettings(const QSettings& settings)
{
    int pid = settings.value(getGroup() + "/client/pid").toInt();
    setDevice(pid);
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

void BloodPressureManager::setDevice(const int& pid)
{
    m_bpm.setConnectionInfo(pid);
    if(armInformationSet() && connectionInfoSet())
    {
        emit message(tr("Ready to connect..."));
        emit canConnectDevice();
    }
}

void BloodPressureManager::setArmBandSize(const QString &size)
{
  m_test.setArmBandSize(size);
  if(armInformationSet() && connectionInfoSet())
  {
      emit message(tr("Ready to connect..."));
      emit canConnectDevice();
  }
}

void BloodPressureManager::setArm(const QString &arm)
{
  m_test.setArm(arm);
  if(armInformationSet() && connectionInfoSet())
  {
      emit message(tr("Ready to connect..."));
      emit canConnectDevice();
  }
}

QJsonObject BloodPressureManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
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
    if("" != firstMeasurement)
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
    if("" != avgMeasurement)
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
    if("" != avgMeasurement)
    {
        QStandardItem* allAvgItem = model->item(row);
        if (Q_NULLPTR == allAvgItem)
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
        return;
    }

    clearData();
    m_bpm.measure();
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

void BloodPressureManager::setupConnections()
{
    connect(&m_bpm, &BPM200::connectionStatusReady, this, &BloodPressureManager::connectionStatusAvailable);
    connect(&m_bpm, &BPM200::measurementReady, this, &BloodPressureManager::measurementAvailable);
    connect(&m_bpm, &BPM200::averageReady, this, &BloodPressureManager::averageAvailable);
    connect(&m_bpm, &BPM200::finalReviewReady, this, &BloodPressureManager::finalReviewAvailable);
    
    // Setup Connections for bpm200
    m_bpm.setupConnections();
}

void BloodPressureManager::clearData()
{
    m_test.reset();
    emit dataChanged();
}

void BloodPressureManager::finish()
{
    m_bpm.disconnect();
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
        // Error message on failure
        emit canMeasure();
        return;
    }
}

void BloodPressureManager::connectionStatusAvailable(const bool& connected)
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
    }
}
