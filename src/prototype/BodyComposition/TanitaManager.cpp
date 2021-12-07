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

// lookup table of default byte arrays for all serial commands
//
QMap<QString,QByteArray> TanitaManager::defaultLUT = TanitaManager::initDefaultLUT();

// lookup table of first two bytes of a request to get the command
//
QMap<QByteArray,QString> TanitaManager::commandLUT = TanitaManager::initCommandLUT();

// lookup table of byte array responses for incorrect inputs
//
QMap<QByteArray,QString> TanitaManager::incorrectLUT= TanitaManager::initIncorrectResponseLUT();

// lookup table of confirmation byte array responses for correct input or request commands
//
QMap<QByteArray,QString> TanitaManager::confirmLUT= TanitaManager::initConfirmationLUT();

TanitaManager::TanitaManager(QObject *parent) : SerialPortManager(parent)
{
}

QMap<QString,QByteArray> TanitaManager::initDefaultLUT()
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
    commands["measure_weight"] = atom;

    atom.clear();
    atom.append(0x1f);
    atom.append(end);
    commands["reset"] = atom;

    return commands;
}

QMap<QByteArray,QString> TanitaManager::initCommandLUT()
{
    QMap<QByteArray,QString> commands;
    QByteArray atom;

    atom = QByteArray("U0");
    commands[atom] = "set_mode_metric";
    atom[1] = '1';
    commands[atom] = "set_mode_imperial";

    atom = QByteArray("R0");
    commands[atom] = "set_equation_westerner";
    atom[1] = '1';
    commands[atom] = "set_equation_oriental";

    atom = QByteArray("D0");
    commands[atom] = "set_clothing_weight";
    atom[1] = '1';
    commands[atom] = "set_gender";
    atom[1] = '2';
    commands[atom] = "set_body_type";
    atom[1] = '3';
    commands[atom] = "set_height";
    atom[1] = '4';
    commands[atom] = "set_age";

    atom = QByteArray("D?");
    commands[atom] = "confirm_settings";

    atom = QByteArray("G1");
    commands[atom] = "measure_body_fat";
    atom[1] = '2';
    commands[atom] = "measure_weight";

    atom.clear();
    atom.append(0x1f);
    atom.append(0x0d);
    commands[atom] = "reset";

    return commands;
}

QMap<QByteArray,QString> TanitaManager::initConfirmationLUT()
{
    QMap<QByteArray,QString> responses;
    QByteArray atom;
    QByteArray end;
    end.append(0x0d);
    end.append(0x0a);

    atom = QByteArray("U0");
    atom.append(end);
    responses[atom] = "correct metric units setting";
    atom[1]='1';
    responses[atom] = "correct imperial units setting";

    atom = QByteArray("R0");
    atom.append(end);
    responses[atom] = "correct equation for Westerners setting";
    atom[1]='1';
    responses[atom] = "correct equation for Orientals setting";

    atom = QByteArray("D0");
    atom.append(end);
    responses[atom] = "correct clothing weight setting";
    atom[1]='1';
    responses[atom] = "correct gender setting";
    atom[1]='2';
    responses[atom] = "correct body type setting";
    atom[1]='3';
    responses[atom] = "correct height setting";
    atom[1]='4';
    responses[atom] = "correct age setting";

    atom = QByteArray("G1");
    atom.append(end);
    responses[atom] = "received body fat measurement request";
    atom[1]='2';
    responses[atom] = "received weight measurement request";

    atom.clear();
    atom.append(0x1f);
    atom.append(end);
    responses[atom] = "received reset request";

    return responses;
}

QMap<QByteArray,QString> TanitaManager::initIncorrectResponseLUT()
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
    responses[atom] = "incorrect clothing (tare) weight setting";

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


void TanitaManager::buildModel(QStandardItemModel *model) const
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

bool TanitaManager::hasEndCode(const QByteArray &arr) const
{
    int size = arr.isEmpty() ? 0 : arr.size();
    if( 2 > size ) return false;
    return (
       0x0d == arr.at(size-2) && //\r
       0x0a == arr.at(size-1) ); //\n
}

void TanitaManager::connectDevice()
{
    if("simulate" == m_mode)
    {
        //m_request = QByteArray("i");
        //writeDevice();
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

      qDebug() << "tanita manager calling resetDevice from connectDevice";
      resetDevice();
    }
}

void TanitaManager::resetDevice()
{
    qDebug() << "reset device called, enqueing command";
    clearQueue();

    m_queue.enqueue(TanitaManager::defaultLUT["reset"]);
    writeDevice();
}

void TanitaManager::confirmSettings()
{
    qDebug() << "***************CONFIRM SETTINGS called *******************";
    m_queue.enqueue(TanitaManager::defaultLUT["confirm_settings"]);
    writeDevice();
}

void TanitaManager::measure()
{
    qDebug() << "measure called";
    m_queue.enqueue(TanitaManager::defaultLUT["measure_body_fat"]);
    writeDevice();
}

// inputs should only be set AFTER a successfuly reset
//
void TanitaManager::setInputs(const QMap<QString,QVariant> &inputs)
{
    qDebug() << "********************* SETTING INPUTS ******************";
    // first lets do a reset

    // units have to be set before tare weight and height have been set
    if(inputs.contains("mode"))
    {
        qDebug() << "enqueue write device for mode " << inputs["mode"].toString();
        QByteArray request = "metric"==inputs["mode"].toString() ?
                    TanitaManager::defaultLUT["set_mode_metric"] :
                    TanitaManager::defaultLUT["set_mode_imperial"];
        m_queue.enqueue(request);
        qDebug() << "enqueed " << request << QString(request);
    }

    // set the equation
    if(inputs.contains("equation"))
    {
        qDebug() << "enqueue write device for equation " << inputs["equation"].toString();
        QByteArray request = "westerner"==inputs["equation"].toString() ?
                    TanitaManager::defaultLUT["set_equation_westerner"] :
                    TanitaManager::defaultLUT["set_equation_oriental"];
        m_queue.enqueue(request);
        qDebug() << "enqueed " << request << QString(request);
    }

    // set the tare weight
    if(inputs.contains("clothing weight"))
    {
        double value = inputs["clothing weight"].toDouble();
        qDebug() << "input clothing weight " << QString::number(value);
        if(0.0 < value && value <= 999.9)
        {
          char buffer[10];
          sprintf(buffer,"%05.1f",value);
          QString s = QString::fromLatin1(buffer);
          QByteArray a = QByteArray::fromStdString(s.toStdString());
          if(5 != a.size())
          {
              qDebug() << "clothing weight input error: " << s << QString(a);
          }
          else
          {
            qDebug() << "enqueue write device for clothing weight " << s;
            QByteArray request = TanitaManager::defaultLUT["set_clothing_weight"];
            request[2] = a[0];
            request[3] = a[1];
            request[4] = a[2];
            request[5] = a[3];
            request[6] = a[4];
            m_queue.enqueue(request);
            qDebug() << "enqueed " << request << QString(request);
          }
        }
    }

    // set gender
    if(inputs.contains("gender"))
    {
        qDebug() << "enqueue write device for gender " << inputs["gender"].toString();
        QByteArray request = "female"==inputs["gender"].toString() ?
                    TanitaManager::defaultLUT["set_gender_female"] :
                    TanitaManager::defaultLUT["set_gender_male"];
        m_queue.enqueue(request);
        qDebug() << "enqueed " << request << QString(request);
    }

    // set body type
    if(inputs.contains("body type"))
    {
        qDebug() << "enqueue write device for body type " << inputs["body type"].toString();
        QByteArray request = "athlete"==inputs["body type"].toString() ?
                    TanitaManager::defaultLUT["set_body_type_athlete"] :
                    TanitaManager::defaultLUT["set_body_type_standard"];
        m_queue.enqueue(request);
    }

    if(inputs.contains("height"))
    {
        int value = inputs["height"].toUInt();
        qDebug() << "input height " << QString::number(value);
        if(0 < value && value <= 99999)
        {
          QString s = QStringLiteral("%1").arg(value,5,10,QLatin1Char('0'));
          QByteArray a = QByteArray::fromStdString(s.toStdString());
          if(5 != a.size())
          {
              qDebug() << "height input error: " << s << QString(a);
          }
          else
          {
            qDebug() << "enqueue write device for height " << s;
            QByteArray request = TanitaManager::defaultLUT["set_height"];
            request[2] = a[0];
            request[3] = a[1];
            request[4] = a[2];
            request[5] = a[3];
            request[6] = a[4];
            m_queue.enqueue(request);
            qDebug() << "enqueed " << request << QString(request);
          }
        }
    }

    if(inputs.contains("age"))
    {
        int value = inputs["age"].toUInt();
        qDebug() << "input age " << QString::number(value);
        if(9 < value && value < 100)
        {
            QByteArray request = TanitaManager::defaultLUT["set_age"];
            QString s = QString::number(value);
            QByteArray a = QByteArray::fromStdString(s.toStdString());
            if(2 != a.size())
            {
                qDebug() << "age input error: " << s;
            }
            else
            {
              qDebug() << "enqueue write device for age " << s;
              request[2] = a[0];
              request[3] = a[1];
              m_queue.enqueue(request);
              qDebug() << "enqueed " << request << QString(request);
            }
        }
        else
            qDebug() << "ERROR: age out of range " << inputs["age"];
    }

    writeDevice();
}

void TanitaManager::clearQueue()
{
    if(!m_queue.isEmpty())
    {
      qDebug() << "clearing queue of " << QString::number(m_queue.size())  << " commands";
      m_queue.clear();
    }
}

void TanitaManager::readDevice()
{
    if("simulate" == m_mode)
    {
    }
    else
    {
      m_buffer += m_port.readAll();
    }
    //if(m_verbose)
    qDebug() << "read device received buffer " << QString(m_buffer);

    qDebug() << (m_port.atEnd() ? "port receiving data ..." : "port finished data receiving ") <<
                " remaining bytes available: " << QString::number(m_port.bytesAvailable()) ;

    QString req;
    if(TanitaManager::commandLUT.contains(m_request.left(2)))
    {
        req = TanitaManager::commandLUT[m_request.left(2)];
        qDebug() << "current request is " << req;
    }
    else
    {
        qDebug() << "unknown byte array request in command LUT " << m_request.left(2) << QString(m_request);
    }

    if(hasEndCode(m_buffer))
    {
      qDebug() << "current request " << req << " end-coded read = " << QString(m_buffer);

      if(TanitaManager::incorrectLUT.contains(m_buffer))
      {
          QString err = TanitaManager::incorrectLUT[m_buffer];
          qDebug() << "incorrect response found " << err;
          emit error(err);
          // if this is an input that failed, try to redo it
          //
          if(req.contains("set_"))
          {
             qDebug() << "*************** RE-ENQUEUE request for " << req;
             m_queue.enqueue(m_request);
          }
          else if("reset" == req)
          {
             emit canInput(false);
          }
          //TODO: set the UI accordingly
      }
      else
      {
          //TODO: set the UI accordingly
          //
          // either we sent a request settings confirmation
          // or
          // we requested a measuremen0 . 0t
          // or
          // we requested a reset
          // or
          // we attempted to set an input
          //
          QString confirmation = TanitaManager::confirmLUT.contains(m_buffer) ?
            TanitaManager::confirmLUT[m_buffer] : QString();

          qDebug() << "confirmation " << confirmation;

          if(req == "confirm_settings")
          {
            qDebug() << "confirming settings in progress ... " << QString::number(m_cache.size()) << " cached items";
            m_cache.push_back(m_buffer);

            // if we are in the middle of confirming, disable UI interaction until fully confirmed
            //
          }
          else if(req == "measure_body_fat")
          {
            // if we have a confirmation, then we are in the middle of processing the measurement request
            // otherwise, check if the buffer is the expected length
            // if the current request is to measure body fat, the response will need to be
            // parsed from 58 bytes in total
            //
            qDebug() << "measure body fat request successful";
          }
          else if(req == "measure_weight")
          {
            // if we have a confirmation, then we are in the middle of processing the measurement request
            // otherwise, check if the buffer is the expected length
            // if the current request is to measure weight, the response will need to be
            // parsed from 6 bytes in total
            //

            qDebug() << "measure weight request successful";
          }
          else if(req == "reset" && !confirmation.isEmpty())
          {
            qDebug() << "reset request successful";
            emit canInput(true);
          }
          else
          {
            qDebug() << "default other command successful " << m_request;

          }
      }

      m_buffer.clear();

      // process the next queued request
      writeDevice();
    }
}

void TanitaManager::writeDevice()
{
  qDebug() << "writeDevice called with remaining bytes to write at " << QString::number(m_port.bytesToWrite());
  // pop off the queue
  if(!m_queue.isEmpty())
  {
    qDebug() << "writeDevice queue size " << QString::number(m_queue.size());
    m_request = m_queue.dequeue();
    if(m_request.isEmpty())
    {
      qDebug() << "ERROR: writeDevice called with empty request in the queue";
    }
    else
    {
      qDebug() << "dequeued request " << TanitaManager::commandLUT[m_request.left(2)] << m_request << QString(m_request);
      qDebug() << (m_port.clear() ? "OK cleared port buffers" : "ERROR: failed to clear port buffers");
      SerialPortManager::writeDevice();
      qDebug() << (m_port.flush() ? "OK flushed" : "ERROR: failed to flush");
    }
  }
  else
  {
      m_request.clear();
  }
}

QJsonObject TanitaManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    json.insert("device",m_deviceData.toJsonObject());
    return json;
}
