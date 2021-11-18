#include "TanitaManager.h"

//#include "../../data/BodyCompositionAnalyzerTest.h"

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QSerialPortInfo>
#include <QSettings>
#include <QtMath>

QMap<QString,QByteArray> TanitaManager::commandLookup = TanitaManager::initCommandLookup();
QMap<QByteArray,QString> TanitaManager::responseLookup= TanitaManager::initResponseLookup();

TanitaManager::TanitaManager(QObject *parent) : SerialPortManager(parent)
{
}

QMap<QString,QByteArray> TanitaManager::initCommandLookup()
{
    QMap<QString,QByteArray> commands;
    QByteArray atom;
    QByteArray end;
    end.append(0x0d);
    end.append(0x0a);

    atom = QByteArray("U0");
    atom.append(end);
    commands["set_mode_metric"] = atom;
    atom[1] = '1';
    commands["set_mode_imperial"] = atom;

    atom[0] = 'R';
    commands["set_equation_oriental"] = atom;
    atom[1] = '0';
    commands["set_equation_westerner"] = atom;

    atom = QByteArray("D0000.0");
    atom.append(end);
    commands["set_clothing_weight"] = atom;
    atom[1] = '3';
    atom[5] = '0';
    commands["set_height"] = atom;

    atom = QByteArray("D11");
    atom.append(end);
    commands["set_gender_male"] = atom;
    atom[2] = '2';
    commands["set_gender_female"] = atom;

    atom[1] = '2';
    atom[2] = '0';
    commands["set_body_type_standard"] = atom;
    atom[2] = '2';
    commands["set_body_type_athlete"] = atom;

    atom = QByteArray("D400");
    atom.append(end);
    commands["set_age"] = atom;

    atom = QByteArray("D?");
    atom.append(end);
    commands["confirm_settings"] = atom;

    atom = QByteArray("G1");
    atom.append(end);
    commands["measure_body_fat"] = atom;
    atom[1] = '2';
    commands["measure_weight_only"] = atom;

    atom.clear();
    atom.append(0x1f);
    atom.append(end);
    commands["reset"] = atom;

    return commands;
}

QMap<QByteArray,QString> TanitaManager::initResponseLookup()
{
    QMap<QByteArray,QString> responses;
    QByteArray atom;
    QByteArray end;
    end.append(0x0d);
    end.append(0x0a);
    atom = QByteArray("U!");
    atom.append(end);
    responses[atom] = "incorrect units setting";
    atom[0] = 'R';
    responses[atom] = "incorrect equation setting";
    atom[0] = 'D';
    responses[atom] = "incorrect tare weight setting";
    atom = QByteArray("D1!");
    atom.append(end);
    responses[atom] = "incorrect gender setting";
    atom[1] = '2';
    responses[atom] = "incorrect body type setting";
    atom[1] = '3';
    responses[atom] = "incorrect height setting";
    atom[1] = '4';
    responses[atom] = "incorrect age setting";
    atom = QByteArray("E00");
    atom.append(end);
    responses[atom] = "attempted to start measuring without completing settings";
    atom[2] = '1';
    responses[atom] = "error in calculating body fat percentage";
    atom[1] = 'X';
    atom[2] = 'X';
    responses[atom] = "other error";
    atom = QByteArray("!");
    atom.append(end);
    responses[atom] = "reset command failed";

    return responses;
}


void TanitaManager::buildModel(QStandardItemModel* model) const
{
    /*
    for(int row = 0; row < m_test.getMaximumNumberOfMeasurements(); row++)
    {
        QString s = "NA";
        WeightMeasurement m = m_test.getMeasurement(row);
        if(m.isValid())
           s = m.toString();
        QStandardItem* item = model->item(row,0);
        item->setData(s, Qt::DisplayRole);
    }
    */
}

void TanitaManager::clearData()
{
    m_deviceData.reset();
    m_test.reset();

    emit dataChanged();
}

bool TanitaManager::hasEndCode(const QByteArray &arr)
{
    int size = arr.size();
    if( 2 > size ) return false;
    return (
       0x0d == arr.at(size-2) && //\r
       0x0a == arr.at(size-1) ); //\n
}

void TanitaManager::connectDevice()
{
    if("simulate" == m_mode)
    {
        m_request = QByteArray("i");
        writeDevice();
        return;
    }

    if(m_port.open(QSerialPort::ReadWrite))
    {
      m_port.setDataBits(QSerialPort::Data8);
      m_port.setParity(QSerialPort::NoParity);
      m_port.setStopBits(QSerialPort::OneStop);
      m_port.setBaudRate(QSerialPort::Baud4800);

      connect(&m_port, &QSerialPort::readyRead,
               this, &TanitaManager::readDevice);

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

      resetDevice();
    }
}

void TanitaManager::resetDevice()
{
    qDebug() << "reset device called";
//    m_request = TanitaManager::commandLookup["reset"];
    m_queue.enqueue(TanitaManager::commandLookup["reset"]);
//    writeDevice();
}

void TanitaManager::confirmSettings()
{
    qDebug() << "confirm settings called";
//    m_request = TanitaManager::commandLookup["confirm_settings"];
    m_queue.enqueue(TanitaManager::commandLookup["confirm_settings"]);
//    writeDevice();
}

void TanitaManager::measure()
{
    qDebug() << "measure called";
//    m_request = TanitaManager::commandLookup["measure_body_fat"];
    m_queue.enqueue(TanitaManager::commandLookup["measure_body_fat"]);
//    writeDevice();
}

void TanitaManager::setInputs(const QMap<QString,QVariant> &inputs)
{
    if(inputs.contains("age"))
    {
        int value = inputs["age"].toUInt();
        if(9 < value && value < 100)
        {
            QByteArray request = TanitaManager::commandLookup["set_age"];
            QString s = QString::number(value);
            QByteArray a = QByteArray::fromStdString(s.toStdString());
            if(2 != a.size())
            {
                qDebug() << "age input error: " << s;
            }
            else
            {
              qDebug() << "write device for age " << s;
              request[2] = a[0];
              request[3] = a[1];
              //writeDevice();
              m_queue.enqueue(request);
            }
        }
    }
    if(inputs.contains("gender"))
    {
        qDebug() << "write device for gender " << inputs["gender"].toString();
        QByteArray request = "female"==inputs["gender"].toString() ?
                    TanitaManager::commandLookup["set_gender_female"] :
                    TanitaManager::commandLookup["set_gender_male"];
        m_queue.enqueue(request);
//        writeDevice();
    }
    if(inputs.contains("body type"))
    {
        qDebug() << "write device for body type " << inputs["body type"].toString();
        QByteArray request = "athlete"==inputs["body type"].toString() ?
                    TanitaManager::commandLookup["set_body_type_athlete"] :
                    TanitaManager::commandLookup["set_body_type_standard"];
        m_queue.enqueue(request);
//        writeDevice();
    }
    if(inputs.contains("height"))
    {
        int value = inputs["height"].toUInt();
        if(0 < value && value <= 99999)
        {
          QString s = QStringLiteral("%1").arg(value,5,10,QLatin1Char('0'));
          QByteArray a = QByteArray::fromStdString(s.toStdString());
          if(5 != a.size())
          {
              qDebug() << "height input error: " << s;
          }
          else
          {
            qDebug() << "write device for height " << s;
            QByteArray request = TanitaManager::commandLookup["set_height"];
            request[2] = a[0];
            request[3] = a[1];
            request[4] = a[2];
            request[5] = a[3];
            request[6] = a[4];
            m_queue.enqueue(request);
//            writeDevice();
          }
        }
    }
    if(inputs.contains("clothing weight"))
    {
        double value = inputs["clothing weight"].toDouble();
        if(0.0 < value && value <= 999.9)
        {
          QString s = QStringLiteral("%1").arg(value,4,'g',1,QLatin1Char('0'));
          QByteArray a = QByteArray::fromStdString(s.toStdString());
          if(5 != a.size())
          {
              qDebug() << "clothing weight input error: " << s;
          }
          else
          {
            qDebug() << "write device for clothing weight " << s;
            QByteArray request = TanitaManager::commandLookup["set_height"];
            request[2] = a[0];
            request[3] = a[1];
            request[4] = a[2];
            request[5] = a[3];
            request[6] = a[4];
            m_queue.enqueue(request);
//            writeDevice();
          }
        }
    }
    if(inputs.contains("equation"))
    {
        qDebug() << "write device for equation " << inputs["equation"].toString();
        QByteArray request = "westerner"==inputs["equation"].toString() ?
                    TanitaManager::commandLookup["set_equation_westerner"] :
                    TanitaManager::commandLookup["set_equation_oriental"];
        m_queue.enqueue(request);
//        writeDevice();
    }
    if(inputs.contains("mode"))
    {
        qDebug() << "write device for mode " << inputs["body type"].toString();
        QByteArray request = "metric"==inputs["mode"].toString() ?
                    TanitaManager::commandLookup["set_mode_metric"] :
                    TanitaManager::commandLookup["set_mode_imperial"];
        m_queue.enqueue(request);
//        writeDevice();
    }
}

void TanitaManager::readDevice()
{
    if("simulate" == m_mode)
    {
    }
    else
    {
      QByteArray data = m_port.readAll();
      m_buffer += data;
    }
    if(m_verbose)
      qDebug() << "read device received buffer " << QString(m_buffer);

    if(!m_buffer.isEmpty() && hasEndCode(m_buffer))
    {
      qDebug() << "current data read " << m_buffer;

      if(TanitaManager::responseLookup.contains(m_buffer))
      {
          qDebug() << "response found " << TanitaManager::responseLookup[m_buffer];
      }

      m_cache.push_back(m_buffer);
      m_buffer.clear();
      QString key = TanitaManager::commandLookup.key(m_request);
      qDebug() << "end code found, end of data stream for current request " << key;
      if("reset"==key)
      {
        emit canConfirm();
      }
      else if("confirm"==key)
      {
         // report all the errors when confirmed
         // of in no errors in the cache emit canMeasure signal
          bool ok = true;
          for(auto&& x : m_cache)
          {
              if(TanitaManager::responseLookup.contains(x))
              {
                  qDebug() << "error response found " << TanitaManager::responseLookup[x];
                  ok = false;
              }
          }
          if(ok)
              emit canMeasure();
      }

    }

}

void TanitaManager::writeDevice()
{
    m_cache.clear();
    SerialPortManager::writeDevice();
}

QJsonObject TanitaManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    json.insert("device",m_deviceData.toJsonObject());
    return json;
}
