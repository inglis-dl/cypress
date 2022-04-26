#include "WeighScaleManager.h"

#include "../data/WeighScaleTest.h"
#include "../auxiliary/Utilities.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QSerialPortInfo>
#include <QSettings>
#include <QStandardItemModel>
#include <QtMath>

WeighScaleManager::WeighScaleManager(QObject* parent) : SerialPortManager(parent)
{
  setGroup("weigh_scale");
  m_col = 1;
  m_row = 2;

  // all managers must check for barcode and language input values
  //
  m_inputKeyList << "barcode";
  m_inputKeyList << "language";

  m_test.setExpectedMeasurementCount(2);
}

void WeighScaleManager::initializeModel()
{
    // allocate 1 column x 2 rows of weight measurement items
    //
    for(int row = 0;row < m_row; row++)
    {
      QStandardItem* item = m_model->item(row,0);
      if(Q_NULLPTR == item)
      {
        item = new QStandardItem();
        m_model->setItem(row,0,item);
      }
      item->setData(QString(), Qt::DisplayRole);
    }
    m_model->setHeaderData(0,Qt::Horizontal,"Weight Tests",Qt::DisplayRole);
}

void WeighScaleManager::updateModel()
{
    if(m_test.isValid())
    {
      for(int row = 0; row < m_row; row++)
      {
        WeightMeasurement measure = m_test.getMeasurement(row);
        if(measure.isValid())
        {
          QString str = measure.toString();
          QStandardItem* item = m_model->item(row,0);
          if(Q_NULLPTR == item)
          {
            item = new QStandardItem();
            m_model->setItem(row,0,item);
          }
          item->setData(str, Qt::DisplayRole);
        }
      }
    }
    else
    {
      initializeModel();
    }
    emit dataChanged();
}

void WeighScaleManager::loadSettings(const QSettings& settings)
{
    QString name = settings.value(getGroup() + "/client/port").toString();
    if(!name.isEmpty())
    {
      setProperty("deviceName", name);
      if(m_verbose)
          qDebug() << "using serial port " << m_deviceName << " from settings file";
    }
}

void WeighScaleManager::saveSettings(QSettings* settings) const
{
    if(!m_deviceName.isEmpty())
    {
      settings->beginGroup(getGroup());
      settings->setValue("client/port",m_deviceName);
      settings->endGroup();
      if(m_verbose)
          qDebug() << "wrote serial port to settings file";
    }
}

void WeighScaleManager::setInputData(const QVariantMap& input)
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
    foreach(const auto key, m_inputKeyList)
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

void WeighScaleManager::clearData()
{
    m_test.reset();
    updateModel();
}

void WeighScaleManager::finish()
{
    m_deviceData.reset();
    m_deviceList.clear();
    m_test.reset();
    if(m_port.isOpen())
        m_port.close();
}

void WeighScaleManager::connectDevice()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        m_request = QByteArray("i");
        writeDevice();
        return;
    }

    if(m_port.isOpen())
        m_port.close();

    if(m_port.open(QSerialPort::ReadWrite))
    {
      m_port.setDataBits(QSerialPort::Data8);
      m_port.setParity(QSerialPort::NoParity);
      m_port.setStopBits(QSerialPort::OneStop);
      m_port.setBaudRate(QSerialPort::Baud9600);

      connect(&m_port, &QSerialPort::readyRead,
               this, &WeighScaleManager::readDevice);

      connect(&m_port, &QSerialPort::errorOccurred,
              this,[this](QSerialPort::SerialPortError error){
                  Q_UNUSED(error)
                  qDebug() << "AN ERROR OCCURED: " << m_port.errorString();
              });

      connect(&m_port, &QSerialPort::dataTerminalReadyChanged,
              this,[](bool set){
          qDebug() << "data terminal ready DTR changed to " << (set?"high":"low");
      });

      connect(&m_port, &QSerialPort::requestToSendChanged,
              this,[](bool set){
          qDebug() << "request to send RTS changed to " << (set?"high":"low");
      });

      // try and read the scale ID, if we can do that then emit the
      // canMeasure signal
      // the canMeasure signal is emitted from readDevice slot on successful read
      //
      m_request = QByteArray("i");
      writeDevice();
    }
}

void WeighScaleManager::zeroDevice()
{
    m_request = QByteArray("z");
    writeDevice();
}

void WeighScaleManager::measure()
{
    m_request = QByteArray("p");
    writeDevice();
}

void WeighScaleManager::readDevice()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        QString simdata;
        if("i" == QString(m_request))
         {
           simdata = "999999";
         }
         else if("z" == QString(m_request))
         {
           simdata = "0.0 kg gross";
         }
         else if("p" == QString(m_request))
         {
            double mu = QRandomGenerator::global()->generateDouble();
            double w = Utilities::interp(77.0f,77.2f,mu);
            simdata = QString("%1 kg gross").arg(QString::number(w,'f',1));
         }
         m_buffer = QByteArray(simdata.toUtf8());
    }
    else
    {
      QByteArray data = m_port.readAll();
      m_buffer += data;
    }
    if(m_verbose)
      qDebug() << "read device received buffer " << QString(m_buffer);

    if(!m_buffer.isEmpty())
    {
      if("i" == QString(m_request))
      {
        m_deviceData.setAttribute("software_id", QString(m_buffer.simplified()));
        // signal the GUI that the measure button can be clicked
        //
        emit message(tr("Ready to measure..."));
        emit canMeasure();
      }
      else if("p" == QString(m_request))
      {
         m_test.fromArray(m_buffer);
         qDebug() << "received p request, read buffer" << m_buffer;
         if(m_test.isValid())
         {
           qDebug() << "test is valid, can save results";
           // emit the can write signal
           emit message(tr("Ready to save results..."));
           emit canWrite();
         }
         else
             qDebug() << "invalid test";
      }
      else if("z" == QString(m_request))
      {
          WeightMeasurement m;
          m.fromArray(m_buffer);
          if(m.isZero())
          {
            // signal the GUI that the measure button can be clicked
            //
            emit message(tr("Ready to measure..."));
            emit canMeasure();
          }
      }

      updateModel();
    }
}

void WeighScaleManager::writeDevice()
{
    // prepare to receive data
    //
    m_buffer.clear();
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        if(m_verbose)
          qDebug() << "in simulate mode writeDevice with request " << QString(m_request);
        readDevice();
        return;
    }

    m_port.write(m_request);
}

QJsonObject WeighScaleManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    json.insert("device",m_deviceData.toJsonObject());
    json.insert("test_input",QJsonObject::fromVariantMap(m_inputData));
    return json;
}
