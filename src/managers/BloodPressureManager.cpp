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
    m_inputKeyList << "barcode";
    m_inputKeyList << "language";
    qDebug() << "Manager created on thread: " << QThread::currentThreadId();
}

void BloodPressureManager::start()
{
    m_bpm.connectToBpm();
    emit dataChanged();
}

void BloodPressureManager::loadSettings(const QSettings& settings)
{
    int vid = settings.value(getGroup() + "/client/vid").toInt();
    int pid = settings.value(getGroup() + "/client/pid").toInt();
    m_bpm.SetConnectionInfo(vid, pid);
}

void BloodPressureManager::saveSettings(QSettings* settings) const
{
}

QJsonObject BloodPressureManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    if ("simulate" != m_mode)
    {
    }
    return json;
}

void BloodPressureManager::buildModel(QStandardItemModel* model) const
{
    // add measurements one row of one columns at a time
    //
    int n_total = m_test.getNumberOfMeasurements();
    int n_row = qMax(1, n_total);
    if (n_row != model->rowCount())
    {
        model->setRowCount(n_row);
    }
    
    // Add first measurement
    int row = 0;
    QString firstMeasurement = m_test.firstMeasurementToString();
    if ("" != firstMeasurement) {
        QStandardItem* firstItem = model->item(row);
        if (Q_NULLPTR == firstItem)
        {
            firstItem = new QStandardItem();
            model->setItem(row, 0, firstItem);
        }
        firstItem->setData(firstMeasurement, Qt::DisplayRole);
        row++;
    }
    

    for (int i = 0; i < n_total; i++)
    {
        BloodPressureMeasurement measurement = m_test.getMeasurement(i);
        QString measurementStr = measurement.isValid() ? measurement.toString() : "NA";
        QStandardItem* item = model->item(row);
        if (Q_NULLPTR == item)
        {
            item = new QStandardItem();
            model->setItem(row, 0, item);
        }
        item->setData(measurementStr, Qt::DisplayRole);
        row++;
    }

    // Add avg measurement
    QString avgMeasurement = m_test.avgMeasurementToString();
    if ("" != avgMeasurement) {
        QStandardItem* avgItem = model->item(row);
        if (Q_NULLPTR == avgItem)
        {
            avgItem = new QStandardItem();
            model->setItem(row, 0, avgItem);
        }
        avgItem->setData(avgMeasurement, Qt::DisplayRole);
        row++;
    }

    // Add all avg measurement
    QString allAvgMeasurement = m_test.allAvgMeasurementToString();
    if ("" != avgMeasurement) {
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
    if ("simulate" == m_mode)
    {
        return;
    }

    clearData();
    // launch the process
    qDebug() << "starting process from measure";
    m_bpm.measure();
}

void BloodPressureManager::setInputData(const QMap<QString, QVariant>& input)
{
    //if ("simulate" == m_mode)
    //{
    //    m_inputData["barcode"] = 12345678;
    //    return;
    //}
    //bool ok = true;
    //for (auto&& x : m_inputKeyList)
    //{
    //    if (!input.contains(x))
    //    {
    //        ok = false;
    //        break;
    //    }
    //    else
    //        m_inputData[x] = input[x];
    //}
    //if (!ok)
    //    m_inputData.clear();
    ////else
    //    // DO SOMETHING
}

void BloodPressureManager::SetupConnections()
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

void BloodPressureManager::connectUI(QWidget*)
{
}

void BloodPressureManager::finish()
{
    m_bpm.disconnect();
    m_test.reset();
}

void BloodPressureManager::measurementAvailable(const int &sbp, const int& dbp, const int& pulse, const const QDateTime& start, 
    const QDateTime& end, const int& readingNum)
{
    m_test.addMeasurement(sbp, dbp, pulse, start, end, readingNum);
    emit dataChanged();
}

void BloodPressureManager::averageAvailable(const int& sbp, const int& dbp, const int& pulse)
{
    m_test.addAverageMeasurement(sbp, dbp, pulse);
    emit dataChanged();
}

void BloodPressureManager::finalReviewAvailable(const int& sbp, const int& dbp, const int& pulse)
{
    bool reviewDataVerified = m_test.verifyReviewData(sbp, dbp, pulse);
    if (reviewDataVerified) {
        emit canWrite();
        return;
    }
    else {
        // Error message on failure
        emit canMeasure();
        return;
    }
}

void BloodPressureManager::connectionStatusAvailable(const bool connected)
{
    if (connected) {
        emit canMeasure();
    }
    else {
        // TODO: Show the following message to user 
        //       "There was a problem connecting to the Blood Pressure Monitor.
        //        Please ensure that the bpm is plugged in, turned on and connected 
        //        to the computer"
    }
}
