#include "BloodPressureManager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>
#include <QStandardItemModel>

BloodPressureManager::BloodPressureManager(QObject* parent) : ManagerBase(parent)
{
    setGroup("bloodpressure");
    m_inputKeyList << "barcode";
}

void BloodPressureManager::start()
{
    emit dataChanged();
}

void BloodPressureManager::loadSettings(const QSettings& settings)
{
}

void BloodPressureManager::saveSettings(QSettings* settings) const
{
}

QJsonObject BloodPressureManager::toJsonObject() const
{
    //QJsonObject json = m_test.toJsonObject();
    QJsonObject json;
    if ("simulate" != m_mode)
    {
        QFile ofile(m_outputFile);
        ofile.open(QIODevice::ReadOnly);
        QByteArray buffer = ofile.readAll();
        json.insert("test_output_file", QString(buffer.toBase64()));
        json.insert("test_output_file_mime_type", "csv");
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
    //    if (nullptr == item)
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
        readOutput();
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

void BloodPressureManager::readOutput()
{
    if ("simulate" == m_mode)
    {
        // TODO: Implement simulate mode
        return;
    }

    /*QDir dir(m_outputPath);
    bool found = false;
    QString fileName = QString("Results-%0.xlsx").arg(m_inputData["barcode"].toString());
    for (auto&& x : dir.entryList())
    {
        if (x == fileName)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        qDebug() << "found output xlsx file " << fileName;
        QString filePath = m_outputPath + QDir::separator() + fileName;
        qDebug() << "found output xlsx file path " << filePath;
        m_test.fromFile(filePath);
        m_outputFile.clear();
        if (m_test.isValid())
        {
            emit canWrite();
            m_outputFile = filePath;
        }
        else
            qDebug() << "ERROR: input from file produced invalid test results";

        emit dataChanged();
    }
    else
        qDebug() << "ERROR: no output csv file found";*/
}

void BloodPressureManager::clearData()
{
    //m_test.reset();
    m_outputFile.clear();
    emit dataChanged();
}

void BloodPressureManager::finish()
{
    //m_test.reset();
}