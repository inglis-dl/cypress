#include "ChoiceReactionManager.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>
#include <QStandardItemModel>

QString ChoiceReactionManager::CCB_PREFIX = "CLSA_ELCV";
QString ChoiceReactionManager::CCB_CLINIC = "CYPRESS";

ChoiceReactionManager::ChoiceReactionManager(QObject *parent) :
    ManagerBase(parent)
{
    setGroup("choice_reaction");
    m_col = 2;
    m_row = 8;

    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode";
    m_inputKeyList << "language";

    //TODO:
    // use the "clinic" CCB arg for the site identification ?
}

void ChoiceReactionManager::start()
{
    // connect signals and slots to QProcess one time only
    //
    connect(&m_process, &QProcess::started,
        this, [this]() {
            qDebug() << "process started: " << m_process.arguments().join(" ");
        });

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &ChoiceReactionManager::readOutput);

    connect(&m_process, &QProcess::errorOccurred,
        this, [](QProcess::ProcessError error)
        {
            QStringList s = QVariant::fromValue(error).toString().split(QRegExp("(?=[A-Z])"), Qt::SkipEmptyParts);
            qDebug() << "ERROR: process error occured: " << s.join(" ").toLower();
        });

    connect(&m_process, &QProcess::stateChanged,
        this, [](QProcess::ProcessState state) {
            QStringList s = QVariant::fromValue(state).toString().split(QRegExp("(?=[A-Z])"), Qt::SkipEmptyParts);
            qDebug() << "process state: " << s.join(" ").toLower();
        });

    m_process.setProcessChannelMode(QProcess::ForwardedChannels);

    configureProcess();
    emit dataChanged();
}

void ChoiceReactionManager::buildModel(QStandardItemModel *model) const
{
    // add measurements one row of two columns at a time
    //
    int n_total = m_test.getNumberOfMeasurements();
    int n_row = qMax(1, n_total/2);
    if(n_row != model->rowCount())
    {
       model->setRowCount(n_row);
    }
    int row_left = 0;
    int row_right = 0;
    for(int i=0; i < n_total; i++)
    {
        QString s = "NA";
        ChoiceReactionMeasurement m = m_test.getMeasurement(i);
        if(m.isValid())
           s = "[" + QString::number(i+1) +"] " + m.toString();
        int col = "left" == m.getAttributeValue("correct_position").toString() ? 0 : 1;
        int *row = 0 == col ? &row_left : &row_right;
        QStandardItem* item = model->item(*row,col);
        if(Q_NULLPTR == item)
        {
          item = new QStandardItem();
          model->setItem(*row,col,item);
        }
        item->setData(s, Qt::DisplayRole);
        (*row)++;
    }
}

bool ChoiceReactionManager::isDefined(const QString &exeName) const
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
       return true;
    }
    bool ok = false;
    if(!exeName.isEmpty())
    {
        QFileInfo info(exeName);
        if(info.exists() && info.isExecutable())
        {
            QString path = info.absolutePath();
            qDebug() << "path to " << exeName << " is " << path;
            QDir dir = QDir::cleanPath(path + QDir::separator() + "results");
            if(dir.exists())
            {
                ok = true;
                qDebug() << "OK: results directory exists " << dir.absolutePath();
            }
            else
                qDebug() << "ERROR: results directory not found " << dir.path();
        }
        else
            qDebug() << "ERROR: info does not exist for file " << exeName;
    }
    else
        qDebug() << "ERROR: isDefined check on empty string";
    return ok;
}

void ChoiceReactionManager::selectRunnable(const QString &exeName)
{
    if(isDefined(exeName))
    {
       QFileInfo info(exeName);
       m_runnableName = exeName;
       m_runnablePath = info.absolutePath();
       QDir dir = QDir::cleanPath(m_runnablePath + QDir::separator() + "results");
       m_outputPath = dir.path();

       emit runnableSelected();
       configureProcess();
    }
    else
       emit canSelectRunnable();
}

void ChoiceReactionManager::setInputData(const QVariantMap& input)
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
    else
      configureProcess();
}

void ChoiceReactionManager::loadSettings(const QSettings &settings)
{
    // the full spec path name including exe name
    // eg., C:\Program Files (x86)\CCB\CCB.exe
    //
    QString exeName = settings.value(getGroup() + "/client/exe").toString();
    selectRunnable(exeName);
}

void ChoiceReactionManager::saveSettings(QSettings *settings) const
{
    if(!m_runnableName.isEmpty())
    {
      settings->beginGroup(getGroup());
      settings->setValue("client/exe",m_runnableName);
      settings->endGroup();
      if(m_verbose)
          qDebug() << "wrote exe fullspec path to settings file";
    }
}

void ChoiceReactionManager::clearData()
{
    m_test.reset();
    m_outputFile.clear();
    emit dataChanged();
}

void ChoiceReactionManager::configureProcess()
{
    if(Constants::RunMode::modeSimulate == m_mode &&
       !m_inputData.isEmpty())
    {
        emit message(tr("Ready to measure..."));
        emit canMeasure();
        return;
    }
    // CCB.exe is present
    //
    QFileInfo info(m_runnableName);
    QDir working(m_runnablePath);
    QDir out(m_outputPath);
    if(info.exists() && info.isExecutable() &&
       working.exists() && out.exists() &&
       !m_inputData.isEmpty())
    {
      qDebug() << "OK: configuring command";

      // the inputs for command line args are present
      QStringList command;
      command << m_runnableName;

      if(m_inputData.contains("interviewer_id"))
         command << "/i" + getInputDataValue("interviewer_id").toString();
      else
         command << "/iNone";

      // minimum required input to identify the file belonging to the participant
      //
      command << "/u" + getInputDataValue("barcode").toString();

      // TODO: consider using upstream host "clinic" identifier
      //
      command << "/c" + CCB_CLINIC;

      // required language "en" or "fr" converted to E or F
      //
      QString s = getInputDataValue("language").toString().toUpper();
      if(!s.isEmpty())
      {
        command << "/l" + QString(s.at(0));
      }

      m_process.setProgram(m_runnableName);
      m_process.setArguments(command);
      m_process.setWorkingDirectory(m_runnablePath);

      qDebug() << "process config args: " << m_process.arguments().join(" ");
      qDebug() << "process working dir: " << m_runnablePath;

      emit message(tr("Ready to measure..."));
      emit canMeasure();
    }
    else
        qDebug() << "failed to configure process";
}

void ChoiceReactionManager::readOutput()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        qDebug() << "simulating read out";
        m_test.reset();
        m_test.addMetaData("start_datetime",QDateTime::currentDateTime());
        m_test.addMetaData("version","simulated");
        m_test.addMetaData("user_id",m_inputData["barcode"].toString());
        m_test.addMetaData("interviewer_id","simulated");
        m_test.addMetaData("end_datetime",QDateTime::currentDateTime());
        m_test.addMetaData("number_of_measurements",60);
        for(int i = 0; i < 60; i++)
        {
            m_test.addMeasurement(ChoiceReactionMeasurement::simulate());
        }
        emit message(tr("Ready to save results..."));
        emit canWrite();
        emit dataChanged();
        return;
    }

    if(QProcess::NormalExit != m_process.exitStatus())
    {
        qDebug() << "ERROR: process failed to finish correctly: cannot read output";
        return;
    }
    else
        qDebug() <<"process finished successfully";

    // Output file name will be of the form
    // <CCB_PREFIX>_<CCB_CLINIC>_YYYYMMDD.csv
    // if CCB_CLINC is not specified at run time with /c option CCB_CLINIC = "Default"
    // YYYYMMDD is the current date.  Multiple runs of CCB.exe therefore overwrite
    // the output file.
    // user id and interviewer id are embedded in the csv file content.
    //
    QStringList pattern;
    pattern << CCB_PREFIX << CCB_CLINIC << QDate().currentDate().toString("yyyyMMdd");
    QString fileName = pattern.join("_") + ".csv";
    QDir dir(m_outputPath);
    if(dir.exists(fileName))
    {
        qDebug() << "found output csv file " << fileName;
        fileName.prepend(QDir::separator());
        fileName.prepend(m_outputPath);
        m_test.fromFile(fileName);
        m_outputFile.clear();
        if(m_test.isValid())
        {
            emit message(tr("Ready to save results..."));
            emit canWrite();
            m_outputFile = fileName;
        }
        else
        {
            // remove the file
            QFile::remove(fileName);
            qDebug() << "ERROR: input from file produced invalid test results";
            qDebug() << "removing " << fileName;
        }

        emit dataChanged();
    }
    else
        qDebug() << "ERROR: no output csv file found";
}

void ChoiceReactionManager::measure()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        readOutput();
        return;
    }

   clearData();
   // launch the process
    qDebug() << "starting process from measure";
    m_process.start();
}

void ChoiceReactionManager::finish()
{
    m_test.reset();
    if(QProcess::NotRunning != m_process.state())
    {
        m_process.close();
    }
    if(!m_outputFile.isEmpty() && QFileInfo::exists(m_outputFile))
    {
      QFile ofile(m_outputFile);
      ofile.remove();
    }
    m_outputFile.clear();
}

QJsonObject ChoiceReactionManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    if(Constants::RunMode::modeSimulate != m_mode)
    {
      QFile ofile(m_outputFile);
      ofile.open(QIODevice::ReadOnly);
      QByteArray buffer = ofile.readAll();
      json.insert("test_output_file",QString(buffer.toBase64()));
      json.insert("test_output_file_mime_type","csv");
    }
    json.insert("test_input",QJsonObject::fromVariantMap(m_inputData));
    return json;
}
