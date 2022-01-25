#include "BodyCompositionAnalyzerManager.h"

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

int BodyCompositionAnalyzerManager::AGE_MIN = 7;
int BodyCompositionAnalyzerManager::AGE_MAX = 99;
int BodyCompositionAnalyzerManager::HEIGHT_MIN_METRIC = 90;
int BodyCompositionAnalyzerManager::HEIGHT_MAX_METRIC = 249;
double BodyCompositionAnalyzerManager::HEIGHT_MIN_IMPERIAL = 36.0;
double BodyCompositionAnalyzerManager::HEIGHT_MAX_IMPERIAL = 95.5;

QByteArray BodyCompositionAnalyzerManager::END_CODE = QByteArray(
  { QChar(QChar::SpecialCharacter::CarriageReturn).toLatin1(),
    QChar(QChar::SpecialCharacter::CarriageReturn).toLatin1() });

// lookup table of default byte arrays for all serial commands
//
QMap<QString,QByteArray> BodyCompositionAnalyzerManager::defaultLUT = BodyCompositionAnalyzerManager::initDefaultLUT();

// lookup table of the unique first two bytes of a request to get the command
//
QMap<QByteArray,QString> BodyCompositionAnalyzerManager::commandLUT = BodyCompositionAnalyzerManager::initCommandLUT();

// lookup table of byte array responses for incorrect inputs
// NOTE: if any of the set input commands are unclear, the response
// from the unit can also be the reset command error
//
QMap<QByteArray,QString> BodyCompositionAnalyzerManager::incorrectLUT= BodyCompositionAnalyzerManager::initIncorrectResponseLUT();

// lookup table of confirmation byte array responses for correct input or request commands
//
QMap<QByteArray,QString> BodyCompositionAnalyzerManager::confirmLUT= BodyCompositionAnalyzerManager::initConfirmationLUT();

BodyCompositionAnalyzerManager::BodyCompositionAnalyzerManager(QObject *parent) : SerialPortManager(parent)
{
  setGroup("body_composition_analyzer");

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

QMap<QString,QByteArray> BodyCompositionAnalyzerManager::initDefaultLUT()
{
    QMap<QString,QByteArray> commands;
    QByteArray atom;

    atom = QByteArray("U0");
    atom.append(END_CODE);
    commands["set_measurement_system_metric"] = atom;
    atom[1] = '1';
    commands["set_measurement_system_imperial"] = atom;

    atom[0] = 'R';
    commands["set_equation_oriental"] = atom;
    atom[1] = '0';
    commands["set_equation_westerner"] = atom;

    atom = QByteArray("D0000.0");
    atom.append(END_CODE);
    commands["set_tare_weight"] = atom;
    atom[1] = '3';
    atom[5] = '0';
    commands["set_height"] = atom;

    atom = QByteArray("D11");
    atom.append(END_CODE);
    commands["set_gender_male"] = atom;
    atom[2] = '2';
    commands["set_gender_female"] = atom;

    atom[1] = '2';
    atom[2] = '0';
    commands["set_body_type_standard"] = atom;
    atom[2] = '2';
    commands["set_body_type_athlete"] = atom;

    atom = QByteArray("D400");
    atom.append(END_CODE);
    commands["set_age"] = atom;

    atom = QByteArray("D?");
    atom.append(END_CODE);
    commands["confirm_settings"] = atom;

    atom = QByteArray("G1");
    atom.append(END_CODE);
    commands["measure_body_fat"] = atom;
    atom[1] = '2';
    commands["measure_weight"] = atom;

    atom.clear();
    atom.append(0x1f);
    atom.append(END_CODE);
    commands["reset"] = atom;

    return commands;
}

QMap<QByteArray,QString> BodyCompositionAnalyzerManager::initCommandLUT()
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
    atom.append(QChar(QChar::SpecialCharacter::CarriageReturn).toLatin1());
    commands[atom] = "reset";

    return commands;
}

QMap<QByteArray,QString> BodyCompositionAnalyzerManager::initConfirmationLUT()
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
    atom.append(QChar(QChar::SpecialCharacter::CarriageReturn).toLatin1());
    responses[atom] = "received reset request";

    return responses;
}

QMap<QByteArray,QString> BodyCompositionAnalyzerManager::initIncorrectResponseLUT()
{
    QMap<QByteArray,QString> responses;
    QByteArray atom;

    atom = QByteArray("U!");
    atom.append(END_CODE);
    responses[atom] = "incorrect measurement system setting";

    atom[0] = 'R';
    responses[atom] = "incorrect equation setting";

    atom[0] = 'D';
    responses[atom] = "incorrect tare (tare) weight setting";

    atom = QByteArray("D1!");
    atom.append(END_CODE);
    responses[atom] = "incorrect gender setting";

    atom[1] = '2';
    responses[atom] = "incorrect body type setting";

    atom[1] = '3';
    responses[atom] = "incorrect height setting";

    atom[1] = '4';
    responses[END_CODE] = "incorrect age setting";

    atom = QByteArray("E00");
    atom.append(END_CODE);
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
    atom.append(END_CODE);
    responses[atom] = "set input or reset command failed";

    return responses;
}

void BodyCompositionAnalyzerManager::loadSettings(const QSettings &settings)
{
    QString name = settings.value(getGroup() + "/client/port").toString();
    if(!name.isEmpty())
    {
      setProperty("deviceName", name);
      if(m_verbose)
          qDebug() << "using serial port " << m_deviceName << " from settings file";
    }
}

void BodyCompositionAnalyzerManager::saveSettings(QSettings *settings) const
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

void BodyCompositionAnalyzerManager::buildModel(QStandardItemModel *model) const
{
    qDebug() << "building model from " <<
                QString::number(m_test.getNumberOfMeasurements()) <<
                "measurements";
    for(int row = 0; row < 8; row++)
    {
        QString s = "NA";
        BodyCompositionMeasurement m = m_test.getMeasurement(row);
        if(m.isValid())
           s = m.toString();
        QStandardItem* item = model->item(row,0);
        if(Q_NULLPTR == item)
        {
           item = new QStandardItem();
           model->setItem(row,0,item);
        }
        item->setData(s, Qt::DisplayRole);
        qDebug() << "model item receiving " << s;
    }
}

void BodyCompositionAnalyzerManager::clearData()
{
    m_deviceData.reset();
    m_test.reset();

    emit dataChanged();
}

void BodyCompositionAnalyzerManager::finish()
{
    m_deviceData.reset();
    m_deviceList.clear();
    m_test.reset();

    if(m_port.isOpen())
        m_port.close();

    m_queue.clear();
    m_cache.clear();
}

bool BodyCompositionAnalyzerManager::hasEndCode(const QByteArray &arr) const
{
    return arr.endsWith(END_CODE);
}

void BodyCompositionAnalyzerManager::connectDevice()
{
    if("simulate" == m_mode)
    {
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
               this, &BodyCompositionAnalyzerManager::readDevice);

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
      resetDevice();
    }
}

void BodyCompositionAnalyzerManager::resetDevice()
{
    qDebug() << "reset device called, enqueing command";
    clearQueue();
    m_queue.enqueue(BodyCompositionAnalyzerManager::defaultLUT["reset"]);
    writeDevice();
}

void BodyCompositionAnalyzerManager::confirmSettings()
{
    qDebug() << "***************CONFIRM SETTINGS called *******************";
    clearQueue();
    // the cache is strictly used for the confirm settings command
    // to store successful responses only.  If the cache reaches
    // 5 confirmed successful inputs, the state changes to ready
    // to measure
    //
    m_cache.clear();
    m_queue.enqueue(BodyCompositionAnalyzerManager::defaultLUT["confirm_settings"]);
    writeDevice();
}

void BodyCompositionAnalyzerManager::measure()
{
    qDebug() << "measure called";
    clearQueue();
    m_queue.enqueue(BodyCompositionAnalyzerManager::defaultLUT["measure_body_fat"]);
    writeDevice();
}

// inputs should only be set AFTER a successful reset
// the order of inputs is important
//
void BodyCompositionAnalyzerManager::setInputData(const QMap<QString,QVariant> &input)
{
    if("simulate" == m_mode)
    {
        m_inputData["barcode"] = "00000000";
        m_inputData["language"] = "english";
        m_inputData["age"] = 50;
        m_inputData["gender"] = "male";
        m_inputData["height"] = 170;
        return;
    }

    qDebug() << "********************* SETTING INPUTS ******************";

    if(input.contains("barcode"))
        m_inputData["barcode"]=input["barcode"];
    if(input.contains("language"))
        m_inputData["language"]=input["language"];

    // for input limits we need the units of measurement
    // default to metric
    //
    QString units = "metric";

    // Units of measurement cannot be set if tare weight and height have been set
    //
    if(input.contains("measurement_system"))
    {
        units = input["measurement_system"].toString().toLower();
        QByteArray request =
          "metric" == units ?
          BodyCompositionAnalyzerManager::defaultLUT["set_measurement_system_metric"] :
          BodyCompositionAnalyzerManager::defaultLUT["set_measurement_system_imperial"];
        m_queue.enqueue(request);
        qDebug() << "enqueued measurement system input " << request << "from " << units;
        m_inputData["measurement_system"] = input["measurement_system"];
    }

    // Set the equation, Westerner or Oriental.  The default setting is the equation for Westerner.
    //
    if(input.contains("equation"))
    {
        QByteArray request =
          "westerner" == input["equation"].toString().toLower() ?
          BodyCompositionAnalyzerManager::defaultLUT["set_equation_westerner"] :
          BodyCompositionAnalyzerManager::defaultLUT["set_equation_oriental"];
        m_queue.enqueue(request);
        qDebug() << "enqueued equation input " << request << "from " << input["equation"].toString();
        m_inputData["measurement_system"] = input["equation"];
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
            QByteArray request = BodyCompositionAnalyzerManager::defaultLUT["set_tare_weight"];
            request[2] = a[0];
            request[3] = a[1];
            request[4] = a[2];
            request[5] = a[3];
            request[6] = a[4];
            m_queue.enqueue(request);
            qDebug() << "enqueued tare weight input " << request << "from " <<
                        QString::number(value);
            m_inputData["tare_weight"] = input["tare_weight"];
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
          BodyCompositionAnalyzerManager::defaultLUT["set_gender_female"] :
          BodyCompositionAnalyzerManager::defaultLUT["set_gender_male"];
        m_queue.enqueue(request);
        qDebug() << "enqueued gender input " << request << "from " << input["gender"].toString();
        m_inputData["gender"] = input["gender"];
    }

    // Set body type.  Once all the necessary items of data have been entered, the unit will
    // determine on the basis of age and gender whether to switch to Athlete or Standard mode.
    // Hence, even if you chose Athlete mode, the unit may switch to Standard mode.
    //
    if(input.contains("body_type"))
    {
        QByteArray request =
          "athlete" == input["body_type"].toString().toLower() ?
          BodyCompositionAnalyzerManager::defaultLUT["set_body_type_athlete"] :
          BodyCompositionAnalyzerManager::defaultLUT["set_body_type_standard"];
        m_queue.enqueue(request);
        qDebug() << "enqueued body type input " << request << "from " << input["body_type"].toString();
        m_inputData["body_type"] = input["body_type"];
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
        if(h_min.toFloat() <= value.toFloat() && value.toFloat() <= h_max.toFloat())
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
            QByteArray request = BodyCompositionAnalyzerManager::defaultLUT["set_height"];
            request[2] = a[0];
            request[3] = a[1];
            request[4] = a[2];
            request[5] = a[3];
            request[6] = a[4];
            m_queue.enqueue(request);
            qDebug() << "enqueued height input " << request << "from " <<
                        value.toString();
            m_inputData["height"] = value;
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
          QByteArray request = BodyCompositionAnalyzerManager::defaultLUT["set_age"];
          QString s = QString::number(value);
          QByteArray a = QByteArray::fromStdString(s.toStdString());
          if(2 == a.size())
          {
            request[2] = a[0];
            request[3] = a[1];
            m_queue.enqueue(request);
            qDebug() << "enqueued age input " << request << "from " <<
                        QString::number(value);
            m_inputData["age"] = input["age"];
          }
          else
            qDebug() << "ERROR: incorrect age format " << s;
        }
        else
          qDebug() << "ERROR: age input out of range " << input["age"];
    }

    // NOTE: validating member var m_inputData is different
    // than other manager implementations since the UI allows for
    // changing the inputs passed from upstream via .json file input
    //
    bool ok = true;
    for(auto&& x : m_inputKeyList)
    {
        if(!m_inputData.contains(x))
        {
            ok = false;
            break;
        }
    }
    if(ok)
    {
        qDebug() << "OK: input data is ok in setInputData()";
        writeDevice();
    }
}

void BodyCompositionAnalyzerManager::clearQueue()
{
    if(!m_queue.isEmpty())
    {
      qDebug() << "clearing queue of " << QString::number(m_queue.size())  << " commands";
      m_queue.clear();
    }
}

void BodyCompositionAnalyzerManager::processResponse(const QByteArray &request, QByteArray response)
{
    if(!hasEndCode(response)) return;
    m_buffer.clear();

    QByteArray commandKey = request.left(2);
    if(!BodyCompositionAnalyzerManager::commandLUT.contains(commandKey))
    {
        qDebug() << "ERROR: failed to process response, unknown command " << commandKey << request;
        return;
    }
    QString requestName = BodyCompositionAnalyzerManager::commandLUT[commandKey];
    qDebug() << "processing response from request " << requestName;

    QByteArray code = response.trimmed();
    if(code.contains("!") || code.contains("E"))
    {
      QString info;
      // see if we can lookup what the issue is
      if(BodyCompositionAnalyzerManager::incorrectLUT.contains(response))
      {
        info = BodyCompositionAnalyzerManager::incorrectLUT[response];
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
      if(BodyCompositionAnalyzerManager::confirmLUT.contains(commandKey))
      {
        info = BodyCompositionAnalyzerManager::confirmLUT[commandKey];
        qDebug() << "CONFIRMED: the command was accepted " << info;
      }
      /*
      else
      {
        // if this is a G measure, then the response will not have a confirm
        if(!request.startsWith("measure_") ||)
           qDebug() << "ERROR: unkown accepted response";
      }
      */
      if(requestName.startsWith("confirm_"))
      {
         m_cache.push_back(response);
         // confirm inputs completed
         if(5 == m_cache.size())
         {
             emit message("Ready to measure...");
             emit canMeasure();
         }
      }
      else if(requestName.startsWith("set_"))
      {
          // if we set the measurement system, set the system used by the test
          //
          if(requestName.startsWith("set_measurement_"))
          {
              QString units =
                "set_measurement_system_metric" == requestName ? "metric" : "imperial";
              m_test.setMeasurementSystem(units);
          }
          emit message("Ready to confirm inputs...");
          emit canConfirm();
      }
      else if(requestName.startsWith("measure_"))
      {
          // clear the buffer and wait for the buffer to fill with the measurement data
          //
          m_buffer.clear();
          qDebug() << "received complete measurement request buffer, size = " << QString::number(response.size());
          if(59 == response.size())
          {
              qDebug() << "passing response to test for parsing ...";
              m_test.fromArray(response);
              if(m_test.isValid())
              {
                  qDebug() << "OK: valid test";
                  emit message("Ready to save results...");
                  emit canWrite();
              }
              else
                  qDebug() << "ERROR: invalid test";
              emit dataChanged();
          }
      }
      else if("reset" == requestName)
      {
          m_test.reset();
          emit message("Ready to accept inputs...");
          emit canInput();
          emit dataChanged();
      }
    }
    writeDevice();
}

void BodyCompositionAnalyzerManager::readDevice()
{
    if("simulate" == m_mode)
    {
        // parent SerialPortManager class calls
        // readDevice for the current request
        // simulate a successful test here
    }
    else
    {
      m_buffer += m_port.readAll();
    }
    //if(m_verbose)
    qDebug() << "read device received buffer " << m_buffer;

    processResponse(m_request, m_buffer);
}

void BodyCompositionAnalyzerManager::writeDevice()
{
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
      qDebug() << "dequeued request " << BodyCompositionAnalyzerManager::commandLUT[m_request.left(2)] << m_request;
      m_buffer.clear();
      m_port.write(m_request);
    }
  }
}

QJsonObject BodyCompositionAnalyzerManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    json.insert("device",m_deviceData.toJsonObject());
    return json;
}

// EXPERIMENTAL ---------------------------------------------------

#include <QComboBox>
#include <QStatusBar>
#include <QMessageBox>
#include <QPushButton>
#include <QApplication>
#include <QLineEdit>
#include <QGroupBox>
#include <QButtonGroup>

void BodyCompositionAnalyzerManager::connectUI(QWidget *widget)
{
    p_widget.reset(widget);
    // Scan for devices
    //
    connect(this, &BodyCompositionAnalyzerManager::scanningDevices,
            p_widget.get(),[this]()
      {
        p_widget->findChild<QComboBox*>("deviceComboBox")->clear();
        p_widget->findChild<QStatusBar*>("statusBar")->showMessage("Discovering serial ports...");
      }
    );

    // Update the drop down list as devices are discovered during scanning
    //
    connect(this, &BodyCompositionAnalyzerManager::deviceDiscovered,
            p_widget.get(), [this](const QString &label){
        QComboBox* ptr = p_widget->findChild<QComboBox*>("deviceComboBox");
        int index = ptr->findText(label);
        if(-1 == index)
        {
            bool oldState = ptr->blockSignals(true);
            ptr->addItem(label);
            ptr->blockSignals(oldState);
        }
    });

    connect(this, &BodyCompositionAnalyzerManager::deviceSelected,
            p_widget.get(),[this](const QString &label){
        QComboBox* ptr = p_widget->findChild<QComboBox*>("deviceComboBox");
        if(label != ptr->currentText())
        {
            ptr->setCurrentIndex(ptr->findText(label));
        }
    });

    // Prompt user to select a device from the drop down list when previously
    // cached device information in the ini file is unavailable or invalid
    //
    connect(this, &BodyCompositionAnalyzerManager::canSelectDevice,
            p_widget.get(),[this](){
        p_widget->findChild<QStatusBar*>("statusBar")->showMessage("Ready to select...");

        QMessageBox::warning(
          p_widget.get(), QApplication::applicationName(),
          tr("Select the port from the list or connect to the currently visible port in the list.  If the device "
          "is not in the list, quit the application and check that the port is "
          "working and connect the analyzer to it before running this application."));
    });

    // Select a device (serial port) from drop down list
    //
    connect(p_widget->findChild<QComboBox*>("deviceComboBox"),
            &QComboBox::currentTextChanged,
            this,&BodyCompositionAnalyzerManager::selectDevice);

    // Ready to connect device
    //
    connect(this, &BodyCompositionAnalyzerManager::canConnectDevice,
            p_widget.get(),[this](){
        p_widget->findChild<QStatusBar*>("statusBar")->showMessage("Ready to connect...");

        for(auto&& x : p_widget->findChildren<QPushButton *>())
            x->setEnabled(false);

        p_widget->findChild<QPushButton*>("connectButton")->setEnabled(true);
        p_widget->findChild<QPushButton*>("closeButton")->setEnabled(true);
    });

    // Connect to device
    //
    connect(p_widget->findChild<QPushButton*>("connectButton"),
            &QPushButton::clicked,
            this, &BodyCompositionAnalyzerManager::connectDevice);

    // Connection is established, inputs are set and confirmed: enable measurement requests
    //
    connect(this, &BodyCompositionAnalyzerManager::canMeasure,
            p_widget.get(),[this](){
        p_widget->findChild<QStatusBar*>("statusBar")->showMessage("Ready to measure...");

        p_widget->findChild<QPushButton*>("resetButton")->setEnabled(true);
        p_widget->findChild<QPushButton*>("setButton")->setEnabled(false);
        p_widget->findChild<QPushButton*>("measureButton")->setEnabled(true);
        p_widget->findChild<QPushButton*>("confirmButton")->setEnabled(true);
        p_widget->findChild<QPushButton*>("saveButton")->setEnabled(false);
    });

    // Connection is established and a successful reset was done
    //
    connect(this, &BodyCompositionAnalyzerManager::canInput,
            p_widget.get(),[this](){
        p_widget->findChild<QStatusBar*>("statusBar")->showMessage("Ready to accept inputs...");

        p_widget->findChild<QPushButton*>("connectButton")->setEnabled(false);
        p_widget->findChild<QPushButton*>("disconnectButton")->setEnabled(true);
        p_widget->findChild<QPushButton*>("resetButton")->setEnabled(true);
        p_widget->findChild<QPushButton*>("setButton")->setEnabled(true);
        p_widget->findChild<QPushButton*>("measureButton")->setEnabled(false);
        p_widget->findChild<QPushButton*>("confirmButton")->setEnabled(false);
        p_widget->findChild<QPushButton*>("saveButton")->setEnabled(false);
    });

    // A successful confirmation of all inputs was done
    //
    connect(this, &BodyCompositionAnalyzerManager::canConfirm,
            p_widget.get(),[this](){
        p_widget->findChild<QStatusBar*>("statusBar")->showMessage("Ready to confirm inputs ...");

        p_widget->findChild<QPushButton*>("connectButton")->setEnabled(false);
        p_widget->findChild<QPushButton*>("disconnectButton")->setEnabled(true);
        p_widget->findChild<QPushButton*>("resetButton")->setEnabled(true);
        p_widget->findChild<QPushButton*>("setButton")->setEnabled(true);
        p_widget->findChild<QPushButton*>("measureButton")->setEnabled(false);
        p_widget->findChild<QPushButton*>("confirmButton")->setEnabled(true);
        p_widget->findChild<QPushButton*>("saveButton")->setEnabled(false);
    });

    // Disconnect from device
    //
    connect(p_widget->findChild<QPushButton*>("disconnectButton"),
            &QPushButton::clicked,
            this, &BodyCompositionAnalyzerManager::disconnectDevice);

    // Reset the device (clear all input settings)
    //
    connect(p_widget->findChild<QPushButton*>("resetButton"),
            &QPushButton::clicked,
            this, &BodyCompositionAnalyzerManager::resetDevice);

    // Set the inputs to the analyzer
    //
    connect(p_widget->findChild<QPushButton*>("setButton"),
            &QPushButton::clicked,
            p_widget.get(),[this](){
        QMap<QString,QVariant> inputs;
        inputs["equation"] = "westerner";

        inputs["measurement system"] =
          "metricRadio" == p_widget->findChild<QButtonGroup*>("unitsGroup")->checkedButton()->objectName() ?
            "metric" : "imperial";

        QString units = inputs["measurement system"].toString();

        inputs["gender"] =
          "maleRadio" == p_widget->findChild<QButtonGroup*>("genderGroup")->checkedButton()->objectName() ?
            "male" : "female";

        QString s = p_widget->findChild<QLineEdit*>("ageLineEdit")->text().simplified();
        s = s.replace(" ","");
        inputs["age"] = s.toUInt();

        inputs["body type"] =
          "standardRadio" == p_widget->findChild<QButtonGroup*>("bodyTypeGroup")->checkedButton()->objectName() ?
            "standard" : "athlete";

        s = p_widget->findChild<QLineEdit*>("heightLineEdit")->text().simplified();
        s = s.replace(" ","");
        inputs["height"] = "metric" == units ? s.toUInt() :  s.toDouble();

        s = p_widget->findChild<QLineEdit*>("tareWeightLineEdit")->text().simplified();
        s = s.replace(" ","");
        inputs["tare weight"] = s.toDouble();

        qDebug() << inputs;

        this->setInputData(inputs);
    });

    // Confirm inputs and check if measurement can proceed
    //
    connect(p_widget->findChild<QPushButton*>("confirmButton"),
            &QPushButton::clicked,
            this, &BodyCompositionAnalyzerManager::confirmSettings);

    // Request a measurement from the device
    //
    connect(p_widget->findChild<QPushButton*>("measureButton"),
            &QPushButton::clicked,
            this, &BodyCompositionAnalyzerManager::measure);
/*
    // Update the UI with any data
    //
    connect(&m_manager, &BodyCompositionAnalyzerManager::dataChanged,
            this,[this](){
        m_manager.buildModel(&m_model);
        int nrow = m_model.rowCount();
        QSize ts_pre = ui->testdataTableView->size();
        ui->testdataTableView->setColumnWidth(0,ui->testdataTableView->size().width()-2);
        ui->testdataTableView->resize(
                    ui->testdataTableView->width(),
                    nrow*(ui->testdataTableView->rowHeight(0)+1)+
                    ui->testdataTableView->horizontalHeader()->height());
        QSize ts_post = ui->testdataTableView->size();
        int dx = ts_post.width()-ts_pre.width();
        int dy = ts_post.height()-ts_pre.height();
        this->resize(this->width()+dx,this->height()+dy);
    });
*/

    // All measurements received: enable write test results
    //
    connect(this, &BodyCompositionAnalyzerManager::canWrite,
            p_widget.get(),[this](){
        p_widget->findChild<QStatusBar*>("statusBar")->showMessage("Ready to save results...");

        p_widget->findChild<QPushButton*>("saveButton")->setEnabled(true);
    });

    QIntValidator *v_age = new QIntValidator(this);
    v_age->setRange(7,99);
    p_widget->findChild<QLineEdit*>("ageLineEdit")->setValidator(v_age);

    // When the units are changed to imperial, the input field for
    // height input must change to accomodate 0.5" increments
    //
    connect(p_widget->findChild<QButtonGroup*>("unitsGroup"),
            QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            p_widget.get(),[this](QAbstractButton *button){
       QLineEdit* ptr =  p_widget->findChild<QLineEdit*>("heightLineEdit");
       ptr->setText("");
       if("metricRadio" == button->objectName())
       {
           QIntValidator *v_ht = new QIntValidator(this);
           v_ht->setRange(90,249);
           ptr->setValidator(v_ht);
       }
       else
       {
           QDoubleValidator *v_ht = new QDoubleValidator(this);
           v_ht->setRange(36.0,7*12 + 11.5);
           v_ht->setDecimals(1);
           ptr->setValidator(v_ht);
       }
    });

    connect(this, &BodyCompositionAnalyzerManager::error,
            p_widget.get(), [this](const QString &error){
        p_widget->findChild<QStatusBar*>("statusBar")->showMessage(error);

        QMessageBox::warning(
          p_widget.get(), QApplication::applicationName(),
          tr("A fatal error occured while attempting a measurement and the "
             "device was disconnected. Turn the device on if it is off, re-connect, "
             "re-input and confirm the inputs."));
    });
}
