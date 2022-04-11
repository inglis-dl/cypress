#include "AudiometerManager.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QSerialPortInfo>
#include <QSettings>
#include <QStandardItemModel>
#include <QtMath>

QByteArray AudiometerManager::END_CODE = AudiometerManager::initEndCode();

QByteArray AudiometerManager::initEndCode()
{
    const char data[] =
      { '~','p','\x17','Z','^',
        QChar(QChar::SpecialCharacter::CarriageReturn).toLatin1() };
    return QByteArray(data);
}

AudiometerManager::AudiometerManager(QObject *parent)
    : SerialPortManager(parent)
{
    setGroup("audiometer");
    m_col = 2;
    m_row = 8;
    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode";
    m_inputKeyList << "language";

    m_test.setMaximumNumberOfMeasurements(16);
}

void AudiometerManager::buildModel(QStandardItemModel* model) const
{
    QStringList sides = {"left","right"};
    foreach(const auto side, sides)
    {
      int index = 0;
      int col = "left" == side ? 0 : 1;
      for(int row = 0; row < m_row; row++)
      {
        HearingMeasurement m = m_test.getMeasurement(side,index);
        if(!m.isValid())
           m.fromCode(side,index,"AA");

        qDebug() << m.toString();

        QStandardItem* item = model->item(row,col);
        item->setData(m.toString(), Qt::DisplayRole);
        index++;
      }
    }
}

void AudiometerManager::loadSettings(const QSettings &settings)
{
    QString name = settings.value(getGroup() + "/client/port").toString();
    if(!name.isEmpty())
    {
      setProperty("deviceName", name);
      selectDevice(m_deviceName);
      if(m_verbose)
          qInfo() << "using serial port setting" << m_deviceName;
    }
}

void AudiometerManager::saveSettings(QSettings* settings) const
{
    if(isDefined(m_deviceName))
    {
      settings->beginGroup(getGroup());
      settings->setValue("client/port",m_deviceName);
      settings->endGroup();
      if(m_verbose)
        qInfo() << "wrote serial port to settings";
    }
}

void AudiometerManager::setInputData(const QVariantMap& input)
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
        qCritical() << "ERROR: missing expected input " << key;
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
        qCritical() << "ERROR: invalid input" << key << value.toString() << QMetaType::typeName(type);
        break;
      }
    }
    if(!ok)
    {
      qCritical() << "ERROR: invalid input data";
      emit message(tr("ERROR: the input data is incorrect"));
      m_inputData = QVariantMap();
    }
}

void AudiometerManager::clearData()
{
    m_test.reset();
    emit dataChanged();
}

void AudiometerManager::finish()
{
    qDebug() << "slot finish";

    if(m_port.isOpen())
        m_port.close();

    m_deviceData.reset();
    m_deviceList.clear();
    m_test.reset();
}

bool AudiometerManager::hasEndCode(const QByteArray &arr)
{
    bool ok = false;
    int len = arr.size() - 1;
    if(5 < len)
    {
      ok = (
        END_CODE.at(5) == arr.at(len) &&
        END_CODE.at(2) == arr.at(len-3) &&
        END_CODE.at(1) == arr.at(len-4) &&
        END_CODE.at(0) == arr.at(len-5)
        );
    }
    return ok;
}

void AudiometerManager::readDevice()
{
    qDebug() << "slot readDevice";

   if(Constants::RunMode::modeSimulate == m_mode)
    {
        QString sim;
        QDateTime now = QDateTime::currentDateTime();
        int m = now.date().month();
        int d = now.date().day();
        int y = now.date().year()-2000;
        int h = now.time().hour();
        int i = now.time().minute();
        char buffer[20];
        sprintf(buffer,"%02d/%02d/%d%02d:%02d:00",m,d,y,h,i);
        QString id = m_inputData["barcode"].toString().leftJustified(14,' ');
        QTextStream(&sim)
          << "\u00010\u00020"
          << id
          << "00055801011124431"
          << buffer
          << "04/01/20000000000       "
          << "35  10  E2  AA  AA  AA  AA  AA  AA  AA  AA  AA  AA  AA  AA  AA       ~p\u0017Z^\r";
        m_buffer = QByteArray(sim.toLatin1());
    }
    else
    {
      QByteArray data = m_port.readAll();
      m_buffer += data;
    }
    if(m_verbose)
      qInfo() << "read device received buffer " << QString(m_buffer);

    if(hasEndCode(m_buffer))
    {
        m_test.fromArray(m_buffer);
        if(m_test.isValid())
        {
            // emit the can write signal
            qDebug() << "signal canWrite";
            emit message(tr("Ready to save results..."));
            emit canWrite();
        }
        emit dataChanged();
    }
}

void AudiometerManager::measure()
{
    qDebug() << "slot measure";
    emit message(tr("Measuring hearing levels..."));
    clearData();
    const char cmd[] = {0x05,'4',0x0d};
    m_request = QByteArray::fromRawData(cmd,3);
    writeDevice();
}

void AudiometerManager::writeDevice()
{
    // prepare to receive data
    //
    m_buffer.clear();
    qDebug() << "slot writeDevice";
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        qDebug() << "simulate writeDevice with request " << QString(m_request);
        readDevice();
        return;
    }

    m_port.write(m_request);
}

QJsonObject AudiometerManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    json.insert("device",m_deviceData.toJsonObject());
    json.insert("test_input",QJsonObject::fromVariantMap(m_inputData));
    return json;
}
