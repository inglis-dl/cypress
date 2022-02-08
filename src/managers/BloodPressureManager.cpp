#include "BloodPressureManager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>
#include <QStandardItemModel>

BloodPressureManager::BloodPressureManager(QObject* parent) 
    : ManagerBase(parent), bpm(new BPM200())
{
    setGroup("bloodpressure");
    m_inputKeyList << "barcode";
    m_inputKeyList << "language";
    qDebug() << "Manager created on thread: " << QThread::currentThreadId();
}

void BloodPressureManager::start()
{
    bpm.Connect();
    // TODO: MOve this to a slot
    //if (connected) {
    //    emit canMeasure();
    //    qDebug() << "Manager: calling Cycle";
    //    bpm.Cycle();
    //    /*qDebug() << "Manager: calling Start";
    //    bpm.Start();*/
    //}
   
    emit dataChanged();
}

void BloodPressureManager::loadSettings(const QSettings& settings)
{
    int vid = settings.value(getGroup() + "/client/vid").toInt();
    int pid = settings.value(getGroup() + "/client/pid").toInt();
    bpm.SetConnectionInfo(vid, pid);
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
    //// add measurements one row of two columns at a time
    ////
    //int n_total = m_test.getNumberOfMeasurements();
    //int n_row = qMax(1, n_total / 2);
    //if (n_row != model->rowCount())
    //{
    //    model->setRowCount(n_row);
    //}
    //int row_left = 0;
    //int row_right = 0;
    //for (int i = 0; i < n_total; i++)
    //{
    //    BloodPressureMeasurement measurement = m_test.getMeasurement(i);
    //    QString measurementStr = measurement.isValid() ? measurement.toString() : "NA";

    //    int col = i % 2;
    //    int* row = col == 0 ? &row_left : &row_right;
    //    QStandardItem* item = model->item(*row, col);
    //    if (Q_NULLPTR == item)
    //    {
    //        item = new QStandardItem();
    //        model->setItem(*row, col, item);
    //    }
    //    item->setData(measurementStr, Qt::DisplayRole);
    //    (*row)++;
    //}
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
    bpm.Disconnect();
    m_test.reset();
}
