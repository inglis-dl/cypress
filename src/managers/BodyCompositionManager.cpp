#include "BodyCompositionManager.h"

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QSerialPortInfo>
#include <QSettings>
#include <QStandardItemModel>
#include <QtMath>

int BodyCompositionManager::AGE_MIN = 7;
int BodyCompositionManager::AGE_MAX = 99;
int BodyCompositionManager::HEIGHT_MIN_METRIC = 90;
int BodyCompositionManager::HEIGHT_MAX_METRIC = 249;
double BodyCompositionManager::HEIGHT_MIN_IMPERIAL = 36.0;
double BodyCompositionManager::HEIGHT_MAX_IMPERIAL = 95.5;

// lookup table of default byte arrays for all serial commands
//
QMap<QString,QByteArray> BodyCompositionManager::defaultLUT = BodyCompositionManager::initDefaultLUT();

// lookup table of the unique first two bytes of a request to get the command
//
QMap<QByteArray,QString> BodyCompositionManager::commandLUT = BodyCompositionManager::initCommandLUT();

// lookup table of byte array responses for incorrect inputs
// NOTE: if any of the set input commands are unclear, the response
// from the unit can also be the reset command error
//
QMap<QByteArray,QString> BodyCompositionManager::incorrectLUT= BodyCompositionManager::initIncorrectResponseLUT();

// lookup table of confirmation byte array responses for correct input or request commands
//
QMap<QByteArray,QString> BodyCompositionManager::confirmLUT= BodyCompositionManager::initConfirmationLUT();

BodyCompositionManager::BodyCompositionManager(QObject *parent) : SerialPortManager(parent)
{
  setGroup("body_composition_analyzer");
  m_col = 1;
  m_row = 8;

  // all managers must check for barcode and language input values
  //
  m_inputKeyList << "barcode";
  m_inputKeyList << "language";

  // mandatory inputs for which there is no default
  // settings having a default are:
  // - measurement_system <metric,imperial>, default = metric
  // - equation <westerner,oriental>, default = westerner
  // - tare_weight, default 0 (kg)
  //
  m_inputKeyList << "height";  // unsigned integer cm
  m_inputKeyList << "age";     // unsigned integer years
  m_inputKeyList << "gender";  // <female,male> or <0,1>
}

// default command lookup by string
// the D series commands for setting tare weight, height, and age
// require specific bytes to be filled with numeric characters
//
QMap<QString,QByteArray> BodyCompositionManager::initDefaultLUT()
{
    QMap<QString,QByteArray> commands;
    QByteArray atom;
    QByteArray end;
    end.append(0x0d);
    end.append(0x0a);

    atom = QByteArray("U0");
    atom.append(end);
    commands["set_measurement_system_metric"] = atom;
    atom[1] = '1';
    commands["set_measurement_system_imperial"] = atom;

    atom[0] = 'R';
    commands["set_equation_oriental"] = atom;
    atom[1] = '0';
    commands["set_equation_westerner"] = atom;

    atom = QByteArray("D0000.0");
    atom.append(end);
    commands["set_tare_weight"] = atom;
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

// the first two characters of a command are sufficient
// to identify what it is
//
QMap<QByteArray,QString> BodyCompositionManager::initCommandLUT()
{
    QMap<QByteArray,QString> commands;
    QByteArray atom;

    atom = QByteArray("U0");
    commands[atom] = "set_measurement_system_metric";
    atom[1] = '1';
    commands[atom] = "set_measurement_system_imperial";

    atom = QByteArray("R0");
    commands[atom] = "set_equation_westerner";
    atom[1] = '1';
    commands[atom] = "set_equation_oriental";

    atom = QByteArray("D0");
    commands[atom] = "set_tare_weight";
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

// the first two characters of a returned buffer are sufficient
// to identify the source command as being confirmated
//
QMap<QByteArray,QString> BodyCompositionManager::initConfirmationLUT()
{
    QMap<QByteArray,QString> responses;
    QByteArray atom;

    atom = QByteArray("U0");
    responses[atom] = "correct metric measurements setting";
    atom[1]='1';
    responses[atom] = "correct imperial measurements setting";

    atom = QByteArray("R0");
    responses[atom] = "correct equation for Westerners setting";
    atom[1]='1';
    responses[atom] = "correct equation for Orientals setting";

    atom = QByteArray("D0");
    responses[atom] = "correct tare weight setting";
    atom[1]='1';
    responses[atom] = "correct gender setting";
    atom[1]='2';
    responses[atom] = "correct body type setting";
    atom[1]='3';
    responses[atom] = "correct height setting";
    atom[1]='4';
    responses[atom] = "correct age setting";

    atom = QByteArray("G1");
    responses[atom] = "received body fat measurement request";
    atom[1]='2';
    responses[atom] = "received weight measurement request";

    atom.clear();
    atom.append(0x1f);
    atom.append(0x0d);
    responses[atom] = "received reset request";

    return responses;
}

// incorrect responses contain "!" character
//
QMap<QByteArray,QString> BodyCompositionManager::initIncorrectResponseLUT()
{
    QMap<QByteArray,QString> responses;
    QByteArray atom;
    QByteArray end;
    end.append(0x0d);
    end.append(0x0a);

    atom = QByteArray("U!");
    atom.append(end);
    responses[atom] = "incorrect measurement system setting";

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
    responses[atom] = "error: attempted to start measuring without completing settings";

    atom[2] = '1';
    responses[atom] = "error: impedance is abnormal compared with height and weight";

    // E-11 - E14
    //
    atom[1] = '1';
    for(int i=1;i<=4;i++)
    {
      atom[2] = QString::number(i).front().toLatin1();
      responses[atom] = "error: internal malfunction, contact Tanita";
    }

    atom[2] = '6';
    responses[atom] = "error: stepped off platform too early, wait for unit to emit short beep";

    atom[2] = '7';
    responses[atom] = "error: stepped on platform too early, wait for flashing arrow to appear";

    atom = QByteArray("!");
    atom.append(end);
    responses[atom] = "set input or reset command failed";

    return responses;
}

void BodyCompositionManager::loadSettings(const QSettings &settings)
{
    QString name = settings.value(getGroup() + "/client/port").toString();
    if(!name.isEmpty())
    {
      setProperty("deviceName", name);
      if(m_verbose)
          qDebug() << "using serial port " << m_deviceName << " from settings file";
    }
}

void BodyCompositionManager::saveSettings(QSettings *settings) const
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

void BodyCompositionManager::buildModel(QStandardItemModel *model) const
{
    QStringList list = m_test.toStringList();
    for(int row = 0; row < list.size(); row++)
    {
      QStandardItem* item = model->item(row,0);
      if(Q_NULLPTR == item)
      {
        item = new QStandardItem();
        model->setItem(row,0,item);
      }
      item->setData(list.at(row), Qt::DisplayRole);
    }
}

void BodyCompositionManager::clearData()
{
    m_deviceData.reset();
    m_test.reset();
    emit dataChanged();
}

void BodyCompositionManager::finish()
{
    m_deviceData.reset();
    m_deviceList.clear();
    m_test.reset();

    if(m_port.isOpen())
        m_port.close();

    m_queue.clear();
    m_cache.clear();
}

bool BodyCompositionManager::hasEndCode(const QByteArray &arr) const
{
    int size = arr.isEmpty() ? 0 : arr.size();
    if( 2 > size ) return false;
    return (
       0x0d == arr.at(size-2) && //\r
       0x0a == arr.at(size-1) ); //\n
}

void BodyCompositionManager::connectDevice()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        resetDevice();
        return;
    }

    if(m_port.isOpen())
        m_port.close();

    if(m_port.open(QSerialPort::ReadWrite))
    {
      m_port.setDataBits(QSerialPort::Data8);
      m_port.setParity(QSerialPort::NoParity);
      m_port.setStopBits(QSerialPort::OneStop);
      m_port.setBaudRate(QSerialPort::Baud4800);

      connect(&m_port, &QSerialPort::readyRead,
               this, &BodyCompositionManager::readDevice);

      connect(&m_port, &QSerialPort::errorOccurred,
              this,[this](QSerialPort::SerialPortError error){
                  if(error == QSerialPort::NoError)
                      return;
                  qDebug() << "ERROR: serial port " << m_port.errorString();
              });

      connect(&m_port, &QSerialPort::dataTerminalReadyChanged,
              this,[](bool set){
          qDebug() << "data terminal ready DTR changed to " << (set?"high":"low");
      });

      connect(&m_port, &QSerialPort::requestToSendChanged,
              this,[](bool set){
          qDebug() << "request to send RTS changed to " << (set?"high":"low");
      });

      // once connected we reset the device to confirm connection was made
      //
      qDebug() << "connect reset device start";
      resetDevice();
      qDebug() << "connect reset device end";
    }
}

void BodyCompositionManager::resetDevice()
{
    qDebug() << "reset device called, enqueing command";
    clearQueue();
    m_queue.enqueue(BodyCompositionManager::defaultLUT["reset"]);
    writeDevice();
}

void BodyCompositionManager::confirmSettings()
{
    qDebug() << "***************CONFIRM SETTINGS called *******************";
    clearQueue();
    // the cache is strictly used for the confirm settings command
    // to store successful responses only.  If the cache reaches
    // 5 confirmed successful inputs, the state changes to ready
    // to measure
    //
    m_cache.clear();
    m_queue.enqueue(BodyCompositionManager::defaultLUT["confirm_settings"]);
    writeDevice();
}

void BodyCompositionManager::measure()
{
    qDebug() << "measure called";
    clearQueue();
    m_queue.enqueue(BodyCompositionManager::defaultLUT["measure_body_fat"]);
    writeDevice();
}

void BodyCompositionManager::setInputData(const QMap<QString,QVariant> &input)
{
    m_inputData = input;
    if(Constants::RunMode::modeSimulate == m_mode)
    {
      if(!input.contains("barcode"))
        m_inputData["barcode"] = Constants::DefaultBarcode;
      if(!input.contains("language"))
        m_inputData["language"] = "english";
      if(!input.contains("age"))
        m_inputData["age"] = 50;
      if(!input.contains("gender"))
        m_inputData["gender"] = "male";
      if(!input.contains("height"))
        m_inputData["height"] = 170;
    }
    bool ok = true;
    foreach(auto key, m_inputKeyList)
    {
      if(!m_inputData.contains(key))
      {
        ok = false;
        if(m_verbose)
          qDebug() << "ERROR: missing expected input " << key;
        break;
      }
    }
    if(!ok)
    {
      if(m_verbose)
        qDebug() << "ERROR: invalid input data";

      emit message(tr("ERROR: the input data is incorrect"));
      m_inputData.clear();
    }
}

// inputs should only be set AFTER a successful reset
// the order of inputs is important
//
void BodyCompositionManager::updateInputData(const QMap<QString,QVariant> &input)
{
    if(input.isEmpty()) return;

    QString units = "metric";

    // Units of measurement cannot be set if tare weight and height have been set
    //
    if(input.contains("measurement_system"))
    {
        units = input["measurement_system"].toString().toLower();
        QByteArray request =
          "metric" == units ?
          BodyCompositionManager::defaultLUT["set_measurement_system_metric"] :
          BodyCompositionManager::defaultLUT["set_measurement_system_imperial"];
        m_queue.enqueue(request);
        qDebug() << "enqueued measurement system input " << request << "from " << units;        
    }

    // Set the equation, Westerner or Oriental.  The default setting is the equation for Westerner.
    //
    if(input.contains("equation"))
    {
        QByteArray request =
          "westerner" == input["equation"].toString().toLower() ?
          BodyCompositionManager::defaultLUT["set_equation_westerner"] :
          BodyCompositionManager::defaultLUT["set_equation_oriental"];
        m_queue.enqueue(request);
        qDebug() << "enqueued equation input " << request << "from " << input["equation"].toString();
    }

    // Set the tare weight. The tare weight need not be set if not required: it will be treated as 0.
    // Tanita BF-350 load cell limits total weight to 200kg or 440lb
    //
    if(input.contains("tare_weight"))
    {
        double value = input["tare_weight"].toDouble();
        double tw_min = 0.0;
        double tw_max = "metric" == units ? 200.0 : 440.0;
        if(tw_min <= value && value <= tw_max)
        {
          char buffer[10];
          sprintf(buffer,"%05.1f",value);
          QString s = QString::fromLatin1(buffer);
          QByteArray a = QByteArray::fromStdString(s.toStdString());
          if(5 == a.size())
          {
            QByteArray request = BodyCompositionManager::defaultLUT["set_tare_weight"];
            request[2] = a[0];
            request[3] = a[1];
            request[4] = a[2];
            request[5] = a[3];
            request[6] = a[4];
            m_queue.enqueue(request);
            qDebug() << "enqueued tare weight input " << request << "from " <<
                        QString::number(value);
          }
          else
            qDebug() << "ERROR: tare weight incorrect format " << s;

        }
        else
          qDebug() << "ERROR: tare weight input out of range " << input["tare_weight"];
    }

    // Set gender
    //
    if(input.contains("gender"))
    {
        QByteArray request =
          (("female" == input["gender"].toString().toLower()) | (0 == input["gender"].toUInt())) ?
          BodyCompositionManager::defaultLUT["set_gender_female"] :
          BodyCompositionManager::defaultLUT["set_gender_male"];
        m_queue.enqueue(request);
        qDebug() << "enqueued gender input " << request << "from " << input["gender"].toString();
    }

    // Set body type.  Once all the necessary items of data have been entered, the unit will
    // determine on the basis of age and gender whether to switch to Athlete or Standard mode.
    // Hence, even if you chose Athlete mode, the unit may switch to Standard mode.
    //
    if(input.contains("body_type"))
    {
        QByteArray request =
          "athlete" == input["body_type"].toString().toLower() ?
          BodyCompositionManager::defaultLUT["set_body_type_athlete"] :
          BodyCompositionManager::defaultLUT["set_body_type_standard"];
        m_queue.enqueue(request);
        qDebug() << "enqueued body type input " << request << "from " << input["body_type"].toString();
    }

    // Set height
    // Tanita BF-350 height limits 90-249cm in 1cm increments
    // or 3ft-7ft11.5in in 0.5in increments
    //
    if(input.contains("height"))
    {
        QVariant value = input["height"];
        QVariant h_min = "metric" == units ? 90.0f : 36.0f;
        QVariant h_max = "metric" == units ? 249.0f : 95.5f;
        if(h_min.toDouble() <= value.toDouble() && value.toDouble() <= h_max.toDouble())
        {
          QString s;
          if("metric" == units)
          {
            s = QStringLiteral("%1").arg(value.toUInt(),5,10,QLatin1Char('0'));
          }
          else
          {
              char buffer[10];
              sprintf(buffer,"%05.1f",value.toDouble());
              s = QString::fromLatin1(buffer);
          }
          QByteArray a = QByteArray::fromStdString(s.toStdString());
          if(5 == a.size())
          {
            QByteArray request = BodyCompositionManager::defaultLUT["set_height"];
            request[2] = a[0];
            request[3] = a[1];
            request[4] = a[2];
            request[5] = a[3];
            request[6] = a[4];
            m_queue.enqueue(request);
            qDebug() << "enqueued height input " << request << "from " <<
                        value.toString();
          }
          else
            qDebug() << "ERROR: incorrect height format " << s;
        }
        else
          qDebug() << "ERROR: height input out of range " <<  input["height"];
    }

    // Set age
    // Tanita BF-350 age limits 7-99 years in 1 year increments
    //
    if(input.contains("age"))
    {
        int value = input["age"].toUInt();
        int a_min = 7;
        int a_max = 99;
        if(a_min <= value && value <= a_max)
        {
          QByteArray request = BodyCompositionManager::defaultLUT["set_age"];
          QString s = QString::number(value);
          QByteArray a = QByteArray::fromStdString(s.toStdString());
          if(2 == a.size())
          {
            request[2] = a[0];
            request[3] = a[1];
            m_queue.enqueue(request);
            qDebug() << "enqueued age input " << request << "from " <<
                        QString::number(value);
          }
          else
            qDebug() << "ERROR: incorrect age format " << s;
        }
        else
          qDebug() << "ERROR: age input out of range " << input["age"];
    }

     qDebug() << "OK: input data is ok in setInputData()";
     writeDevice();
}

void BodyCompositionManager::clearQueue()
{
    if(!m_queue.isEmpty())
    {
      qDebug() << "clearing queue of " << QString::number(m_queue.size())  << " commands";
      m_queue.clear();
    }
}

void BodyCompositionManager::processResponse()
{
    if(!hasEndCode(m_buffer)) return;
    QByteArray request = m_request;
    QByteArray response = m_buffer;
    m_buffer.clear();

    QByteArray commandKey = request.left(2);
    if(!BodyCompositionManager::commandLUT.contains(commandKey))
    {
        qDebug() << "ERROR: failed to process response, unknown command " << commandKey << m_request;
        return;
    }
    QString requestName = BodyCompositionManager::commandLUT[commandKey];
    qDebug() << "processing response from request " << requestName;

    QByteArray code = response.trimmed();
    if(code.contains("!") || code.contains("E"))
    {
      QString info;
      // lookup the issue
      //
      if(BodyCompositionManager::incorrectLUT.contains(response))
      {
        info = BodyCompositionManager::incorrectLUT[response];
        qDebug() << "WARNING: the request produced an invalid response " << info;
      }
      else
      {
        qDebug() << "WARNING: unknown invalid response";
      }
      if(code.contains("!"))
      {
        // if this is a set_ type of command retry by pushing the request back onto the queue
        if(requestName.startsWith("set_"))
        {
          m_queue.enqueue(request);
        }
        else if("reset" == requestName)
        {
          m_queue.enqueue(request);
        }
        else if(requestName.startsWith("confirm_"))
        {
          // continue reading the buffer
          return;
        }
      }
      else if(code.startsWith("E"))
      {
        // abort all further processing
        clearQueue();

        // report the error and stop all processing
        // most E cases will shut off the device requiring power up
        // and reconnection
        //
        emit error(info);
        disconnectDevice();
        return;
      }
    }
    else
    {
      // do we have a successful response?
      QString info;
      if(BodyCompositionManager::confirmLUT.contains(commandKey))
      {
        info = BodyCompositionManager::confirmLUT[commandKey];
        qDebug() << "CONFIRMED: the command was accepted " << info;
      }
      if(requestName.startsWith("confirm_"))
      {
         if(Constants::RunMode::modeSimulate == m_mode)
         {
           emit message(tr("Ready to measure..."));
           emit canMeasure();
         }
         else
         {
           m_cache.push_back(response);
           // confirm inputs completed
           if(5 == m_cache.size())
           {
             emit message(tr("Ready to measure..."));
             emit canMeasure();
           }
         }
      }
      else if(requestName.startsWith("set_"))
      {
          // if we set the measurement system, set the system used by the test
          //
          if(requestName.startsWith("set_measurement_"))
          {
              Constants::UnitsSystem units =
                "set_measurement_system_metric" == requestName ?
                Constants::UnitsSystem::systemMetric :
                Constants::UnitsSystem::systemImperial;
              m_test.setUnitsSystem(units);
          }
          emit message(tr("Ready to confirm inputs..."));
          emit canConfirm();
      }
      else if(requestName.startsWith("measure_"))
      {
          if(Constants::RunMode::modeSimulate == m_mode)
          {
            m_test.simulate(
              m_inputData["age"].toDouble(),
              m_inputData["gender"].toString(),
              m_inputData["height"].toDouble());
          }
          else
          {
            qDebug() << "received complete measurement request buffer, size = " << QString::number(response.size());
            if(59 == response.size())
            {
              qDebug() << "passing response to test for parsing ...";
              m_test.fromArray(response);
            }
          }
          if(m_test.isValid())
          {
            qDebug() << "OK: valid test";
            emit message(tr("Ready to save results..."));
            emit canWrite();
           }
           else
             qDebug() << "ERROR: invalid test";

          emit dataChanged();
      }
      else if("reset" == requestName)
      {
          m_test.reset();
          emit message(tr("Ready to accept inputs..."));
          emit canInput();
          emit dataChanged();
      }
    }
    writeDevice();
}

void BodyCompositionManager::readDevice()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        // parent SerialPortManager class calls
        // readDevice for the current request
        // simulate a successful test here
        QByteArray end;
        end.append(0x0d);
        end.append(0x0a);
        m_buffer = end;
        qDebug() << "simulated buffer with end code" << m_buffer.toHex();
    }
    else
    {
      qDebug() << "readDevice adding to buffer from port";
      m_buffer += m_port.readAll();
    }
    if(m_verbose)
      qDebug() << "read device received buffer " << m_buffer;

    qDebug() <<"readDevice process response start";
    processResponse();
    qDebug() <<"readDevice process response end";
}

void BodyCompositionManager::writeDevice()
{
  if(!m_queue.isEmpty())
  {
    if(m_verbose)
      qDebug() << "writeDevice queue size" << QString::number(m_queue.size()) <<"request";
    m_request = m_queue.dequeue();
    if(m_request.isEmpty())
    {
      if(m_verbose)
        qDebug() << "ERROR: writeDevice called with empty request in the queue";
    }
    else
    {
      if(m_verbose)
        qDebug() << "dequeued request " << BodyCompositionManager::commandLUT[m_request.left(2)] << m_request.toHex();
      m_buffer.clear();

      if(Constants::RunMode::modeSimulate == m_mode)
      {
          //TODO: simulate buffer content based on current request
          //
          if(m_verbose)
            qDebug() << "processing simulated request";
          readDevice();
          return;
      }
      qDebug() << "writeDevice port writing request start";
      m_port.write(m_request);
      qDebug() << "writeDevice port writing request end";
    }
  }
}

QJsonObject BodyCompositionManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    json.insert("device",m_deviceData.toJsonObject());
    return json;
}
