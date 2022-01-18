#include "ChoiceReactionManager.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>
#include <QStandardItemModel>

ChoiceReactionManager::ChoiceReactionManager(QObject *parent) :
    ManagerBase(parent)
{
    setGroup("choice_reaction");

    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode";
    m_inputKeyList << "language";

    //TODO:
    // use the "clinic" CCB arg for the site identification
}

void ChoiceReactionManager::start()
{
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
        int col = "left" == m.getCharacteristic("correct position") ? 0 : 1;
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
    if("simulate" == m_mode)
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

       configureProcess();
    }
    else
       emit canSelectRunnable();
}

void ChoiceReactionManager::setInputData(const QMap<QString, QVariant> &input)
{
    if("simulate" == m_mode)
    {
        m_inputData["barcode"] = "00000000";
        m_inputData["language"] = "english";
        m_inputData["interviewer_id"] = "simi";
        m_inputData["clinic"] = "simc";
        return;
    }
    bool ok = true;
    for(auto&& x : m_inputKeyList)
    {
        if(!input.contains(x))
        {
            ok = false;
            break;
        }
        else
            m_inputData[x] = input[x];
    }
    if(!ok)
        m_inputData.clear();
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
    if("simulate" == m_mode &&
       !m_inputData.isEmpty())
    {
        emit canMeasure();
        return;
    }
    // the CCB.exe is present
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
         command << "/u" + getInputDataValue("interviewer_id").toString();

      // minimum required input to identify the file belonging to the participant
      //
      command << "/i" + getInputDataValue("barcode").toString();

      // currently do not require a site name
      if(m_inputData.contains("clinic"))
         command << "/c" + getInputDataValue("clinic").toString();

      // required language "english" or "french" converted to En or Fr
      QString s = getInputDataValue("language").toString().toLower();
      if(!s.isEmpty())
      {
        s = s.at(0).toUpper() + s.mid(1);
        command << "/l" + s;
      }

      m_process.setProcessChannelMode(QProcess::ForwardedChannels);
      m_process.setProgram(m_runnableName);
      m_process.setArguments(command);
      m_process.setWorkingDirectory(m_runnablePath);

      qDebug() << "process config args: " << m_process.arguments().join(" ");
      qDebug() << "process working dir: " << m_runnablePath;

      connect(&m_process,&QProcess::started,
              this,[this](){
          qDebug() << "process started: " << m_process.arguments().join(" ");
      });

      connect(&m_process,QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
              this, &ChoiceReactionManager::readOutput);

      connect(&m_process,&QProcess::errorOccurred,
              this, [](QProcess::ProcessError error)
      {
          QStringList s = QVariant::fromValue(error).toString().split(QRegExp("(?=[A-Z])"),Qt::SkipEmptyParts);
          qDebug() << "ERROR: process error occured: " << s.join(" ").toLower();
      });

      connect(&m_process,&QProcess::stateChanged,
              this,[](QProcess::ProcessState state){
          QStringList s = QVariant::fromValue(state).toString().split(QRegExp("(?=[A-Z])"),Qt::SkipEmptyParts);
          qDebug() << "process state: " << s.join(" ").toLower();

      });

      emit canMeasure();
    }
    else
        qDebug() << "failed to configure process";
}

void ChoiceReactionManager::readOutput()
{
    if("simulate" == m_mode)
    {
        qDebug() << "simulating read out";
        m_test.addMetaDataCharacteristic("start_datetime",QDateTime::currentDateTime());
        m_test.reset();
        for(int i = 0; i < 60; i++)
        {
            m_test.addMeasurement(ChoiceReactionMeasurement::simulate());
        }
        m_test.addMetaDataCharacteristic("version","simulated");
        m_test.addMetaDataCharacteristic("user_id","00000000");
        m_test.addMetaDataCharacteristic("interviewer_id","00000000");
        m_test.addMetaDataCharacteristic("end_datetime",QDateTime::currentDateTime());
        m_test.addMetaDataCharacteristic("number_of_measurements",60);
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

    QDir dir(m_outputPath);
    bool found = false;
    QString fileName;

    // the file name will be of the form CLSA_ELCV_InterviewId_YYYYMMDD.csv
    // the UserId is mandatory but we use it to identify the participant with the barcode
    // the InterviewerId is either "Default" or the command line argument for
    // the interviewer id
    // to uniquely identify files by name when multiple tests are run in one day
    // we will use the user id as the interviewer id and the interviewer id
    // as the barcode
    //
    for(auto&& x : dir.entryList())
    {
        // TODO: verify the file contains the data for the
        // prescribed participant ID
        //
        if (x.endsWith(".csv") &&
            x.contains(getInputDataValue("barcode").toString()))
        {
            fileName = x;
            found = true;
            break;
        }
    }
    if(found)
    {
        qDebug() << "found output csv file " << fileName;
        fileName = m_outputPath + QDir::separator() + fileName;
        m_test.fromFile(fileName);
        m_outputFile.clear();
        if(m_test.isValid())
        {
            emit canWrite();
            m_outputFile = fileName;
        }
        else
            qDebug() << "ERROR: input from file produced invalid test results";

        emit dataChanged();
    }
    else
        qDebug() << "ERROR: no output csv file found";
}

void ChoiceReactionManager::measure()
{
    if(!m_validBarcode)
    {
        qDebug() << "ERROR: barcode has not been validated";
        return;
    }
    if("simulate" == m_mode)
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
    if("simulate" != m_mode)
    {
      QFile ofile(m_outputFile);
      ofile.open(QIODevice::ReadOnly);
      QByteArray buffer = ofile.readAll();
      json.insert("test_output_file",QString(buffer.toBase64()));
      json.insert("test_output_file_mime_type","csv");
    }
    return json;
}
