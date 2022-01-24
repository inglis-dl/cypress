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

AudiometerManager::AudiometerManager(QObject *parent) : SerialPortManager(parent)
{
    setGroup("audiometer");

    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode";
    m_inputKeyList << "language";
}

void AudiometerManager::buildModel(QStandardItemModel* model) const
{
    QVector<QString> v_side({"left","right"});
    for(auto&& side : v_side)
    {
      int i_freq = 0;
      int col = "left" == side ? 0 : 1;
      for(int row=0;row<8;row++)
      {
        HearingMeasurement m = m_test.getMeasurement(side,i_freq);
        if(!m.isValid())
           m.fromCode(side,i_freq,"AA");
        QStandardItem* item = model->item(row,col);
        item->setData(m.toString(), Qt::DisplayRole);
        i_freq++;
      }
    }
}

void AudiometerManager::loadSettings(const QSettings &settings)
{
    QString name = settings.value(getGroup() + "/client/port").toString();
    if(!name.isEmpty())
    {
      setProperty("deviceName", name);
      if(m_verbose)
          qDebug() << "using serial port " << m_deviceName << " from settings file";
    }
}

void AudiometerManager::saveSettings(QSettings *settings) const
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

void AudiometerManager::setInputData(const QMap<QString, QVariant> &input)
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
            qDebug() << "ERROR: missing expected input " << x;
            break;
        }
    }
    if(!ok)
        m_inputData.clear();
}

void AudiometerManager::clearData()
{
    m_test.reset();
    emit dataChanged();
}

void AudiometerManager::finish()
{
    m_deviceData.reset();
    m_deviceList.clear();
    m_test.reset();
    if(m_port.isOpen())
        m_port.close();
}

bool AudiometerManager::hasEndCode(const QByteArray &arr)
{
    return arr.endsWith(END_CODE);
}

void AudiometerManager::readDevice()
{
   if("simulate" == m_mode)
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
        QTextStream(&sim)
          << "\u00010\u0002000000000      "
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
      qDebug() << "read device received buffer " << QString(m_buffer);

    if(hasEndCode(m_buffer))
    {
        m_test.fromArray(m_buffer);
        if(m_test.isValid())
        {
            // emit the can write signal
            emit message(tr("Ready to write..."));
            emit canWrite();
        }

        emit dataChanged();
    }
}

void AudiometerManager::measure()
{
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

    if("simulate" == m_mode)
    {
        if(m_verbose)
          qDebug() << "in simulate mode writeDevice with request " << QString(m_request);
        readDevice();
        return;
    }

    m_port.write(m_request);
}

QJsonObject AudiometerManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    json.insert("device",m_deviceData.toJsonObject());
    return json;
}
