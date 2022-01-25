#include "TemplateManager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>
#include <QStandardItemModel>

TemplateManager::TemplateManager(QObject* parent) : ManagerBase(parent)
{
    setGroup("template");

    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode";
    m_inputKeyList << "language";
}

void TemplateManager::start()
{
    emit dataChanged();
}

void TemplateManager::loadSettings(const QSettings& settings)
{
}

void TemplateManager::saveSettings(QSettings* settings) const
{
}

QJsonObject TemplateManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    if("simulate" != m_mode)
    {
        // Simulate mode code
    }
    QJsonObject jsonInput;
    for(auto&& x : m_inputData.toStdMap())
    {
        // convert to space delimited phrases to snake_case
        //
        jsonInput.insert(QString(x.first).toLower().replace(QRegExp("[\\s]+"),"_"), QJsonValue::fromVariant(x.second));
    }
    json.insert("test_input",jsonInput);
    return json;
}

void TemplateManager::buildModel(QStandardItemModel* model) const
{
    // add measurements one row of two columns at a time
    //
    int n_total = m_test.getNumberOfMeasurements();
    int n_row = qMax(1, n_total / 2);
    if (n_row != model->rowCount())
    {
        model->setRowCount(n_row);
    }
    int row_left = 0;
    int row_right = 0;
    for (int i = 0; i < n_total; i++)
    {
        TemplateMeasurement measurement = m_test.getMeasurement(i);
        QString measurementStr = measurement.isValid() ? measurement.toString() : "NA";

        int col = i % 2;
        int* row = col == 0 ? &row_left : &row_right;
        QStandardItem* item = model->item(*row, col);
        if (nullptr == item)
        {
            item = new QStandardItem();
            model->setItem(*row, col, item);
        }
        item->setData(measurementStr, Qt::DisplayRole);
        (*row)++;
    }
}

void TemplateManager::measure()
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

void TemplateManager::setInputData(const QMap<QString, QVariant>& input)
{
    if("simulate" == m_mode)
    {
        m_inputData["barcode"] = "00000000";
        m_inputData["language"] = "english";
        return;
    }
    bool ok = true;
    m_inputData = input;
    for(auto&& x : m_inputKeyList)
    {
        if(!input.contains(x))
        {
            ok = false;
            break;
        }
    }
    if(!ok)
        m_inputData.clear();
    else
    {
    // DO SOMETHING
    }
}

void TemplateManager::readOutput()
{
    if("simulate" == m_mode)
    {
        // TODO: Implement simulate mode
        return;
    }

    // Read the output for non simulate mode
}

void TemplateManager::clearData()
{
    m_test.reset();
    emit dataChanged();
}

void TemplateManager::finish()
{
    m_test.reset();
}
