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
    m_col = 1;
    m_row = 1;

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
    Q_UNUSED(settings)
}

void TemplateManager::saveSettings(QSettings* settings) const
{
    Q_UNUSED(settings)
}

QJsonObject TemplateManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        // simulate mode code
    }
    json.insert("test_input",QJsonObject::fromVariantMap(m_inputData));
    return json;
}

void TemplateManager::buildModel(QStandardItemModel* model) const
{
    // example of a 1 row 1 column model displaying one measurement
    //
    for(int row = 0; row < m_test.getNumberOfMeasurements(); row++)
    {
        QStandardItem* item = model->item(row, 0);
        if(Q_NULLPTR == item)
        {
            item = new QStandardItem();
            model->setItem(row, 0, item);
        }
        item->setData(m_test.getMeasurement(row).toString(), Qt::DisplayRole);
    }
}

void TemplateManager::measure()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        return;
    }

    clearData();
    // launch the process
    if(m_verbose)
      qDebug() << "starting process from measure";
}

void TemplateManager::setInputData(const QVariantMap& input)
{
    m_inputData = input;
    if(Constants::RunMode::modeSimulate == m_mode)
    {
      if(!input.contains("barcode"))
        m_inputData["barcode"] = Constants::DefaultBarcode;
      if(!input.contains("language"))
        m_inputData["language"] = "english";
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
      const QVariant value = m_inputData[key];
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
      m_inputData = QVariantMap();
    }
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
