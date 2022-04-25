#include "GripStrengthManager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>
#include <QStandardItemModel>

GripStrengthManager::GripStrengthManager(QObject* parent) : ManagerBase(parent)
{
    setGroup("gripstrength");
    m_col = 1;
    m_row = 1;

    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode";
}

void GripStrengthManager::start()
{
    emit dataChanged();
    QString testPath = "C:/Users/clsa/Desktop/ZGripTest.DB";
    QString testDataPath = "C:/Users/clsa/Desktop/ZGripTestData.DB";
    m_test.fromParradox(testPath, testDataPath);
}

void GripStrengthManager::loadSettings(const QSettings& settings)
{
    Q_UNUSED(settings)
}

void GripStrengthManager::saveSettings(QSettings* settings) const
{
    Q_UNUSED(settings)
}

QJsonObject GripStrengthManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        // simulate mode code
    }
    json.insert("test_input",QJsonObject::fromVariantMap(m_inputData));
    return json;
}

void GripStrengthManager::buildModel(QStandardItemModel* model) const
{
    // example of a 1 row 1 column model displaying one measurement
    //
    for(int row = 0; row < m_test.getMeasurementCount(); row++)
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

void GripStrengthManager::measure()
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

void GripStrengthManager::setInputData(const QVariantMap& input)
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

void GripStrengthManager::clearData()
{
    m_test.reset();
    emit dataChanged();
}

void GripStrengthManager::finish()
{
    m_test.reset();
}
