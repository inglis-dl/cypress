#include "CognitiveTestManager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>

CognitiveTestManager::CognitiveTestManager(QObject *parent) :
    ManagerBase(parent)
{
}

void CognitiveTestManager::buildModel(QStandardItemModel *model) const
{
    // add measurements one row of two columns at a time
    //
    int n_total = m_test.getNumberOfMeasurements();
    int n_row = qMax(1, n_total/2);
    int n_used = 0;
    int rowCount = model->rowCount();
    if(rowCount!=n_row)
    {
       model->setRowCount(n_row);
    }

    for(int row = 0; row < n_row; row++)
    {
        for(int col = 0; col < 2; col++)
        {
          QString s = "NA";
          ChoiceReactionMeasurement m = m_test.getMeasurement(n_used++);
          if(m.isValid())
             s = m.toString();
           QStandardItem* item = model->item(row,col);
           if(nullptr == item)
           {
             item = new QStandardItem();
             model->setItem(row,col,item);
           }
           item->setData(s, Qt::DisplayRole);
         }
    }
}

bool CognitiveTestManager::isDefined(const QString &exeName) const
{
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

void CognitiveTestManager::setExecutableName(const QString &exeName)
{
    if(isDefined(exeName))
    {
       QFileInfo info(exeName);
       m_executableName = exeName;
       m_executablePath = info.absolutePath();
       QDir dir = QDir::cleanPath(m_executablePath + QDir::separator() + "results");
       m_outputPath = dir.path();

       QMap<QString,QVariant> inputs;
       inputs["barcode"] = 12345678;
       inputs["language"] = "english";
       inputs["user"] = "simulate_interviewer";
       inputs["site"] = "simulate_dcs";
       setInputs(inputs);
    }
}

void CognitiveTestManager::loadSettings(const QSettings &settings)
{
    // the full spec path name including exe name
    // eg., C:\Program Files (x86)\CCB\CCB.exe
    //
    QString exeName = settings.value("client/exe").toString();
    qDebug() << "loading settings with exe " << exeName;
    setExecutableName(exeName);
}

void CognitiveTestManager::saveSettings(QSettings *settings) const
{
    if(!m_executableName.isEmpty())
    {
      settings->setValue("client/exe",m_executableName);
      if(m_verbose)
          qDebug() << "wrote exe fullspec path to settings file";
    }
}

void CognitiveTestManager::clearData()
{
    m_test.reset();
    m_outputFile.clear();
    emit dataChanged();
}

void CognitiveTestManager::configureProcess()
{
    // the exe is present
    QFileInfo info(m_executableName);
    QDir working(m_executablePath);
    QDir out(m_outputPath);
    if(info.exists() && info.isExecutable() &&
       working.exists() && out.exists())
    {
      qDebug() << "OK: configuring command";

      // the inputs for command line args are present
      QStringList command;
      command << m_executableName;

      if(m_inputData.contains("barcode"))
         command << "/i" + m_inputData["barcode"].toString();

      if(m_inputData.contains("site"))
         command << "/c" + m_inputData["site"].toString();

      if(m_inputData.contains("user"))
         command << "/u" + m_inputData["user"].toString();

      if(m_inputData.contains("language"))
      {
         QString s = m_inputData["language"].toString().toLower();
         s = s.at(0).toUpper() + s.mid(1);
         command << "/l" + s;
      }
      m_process.setProcessChannelMode(QProcess::ForwardedChannels);
      m_process.setProgram(m_executableName);
      m_process.setArguments(command);
      m_process.setWorkingDirectory(m_executablePath);

      qDebug() << "process config args: " << m_process.arguments().join(" ");
      qDebug() << "process working dir: " << m_executablePath;

      connect(&m_process,&QProcess::started,
              this,[this](){
          qDebug() << "process started: " << m_process.arguments().join(" ");
      });

      connect(&m_process,QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
              this, &CognitiveTestManager::readOutput);

      connect(&m_process,&QProcess::errorOccurred,
              this, [](QProcess::ProcessError error)
      {
          QStringList s = QVariant::fromValue(error).toString().split(QRegExp("(?=[A-Z])"),QString::SkipEmptyParts);
          qDebug() << "ERROR: process error occured: " << s.join(" ").toLower();
      });

      connect(&m_process,&QProcess::stateChanged,
              this,[](QProcess::ProcessState state){
          QStringList s = QVariant::fromValue(state).toString().split(QRegExp("(?=[A-Z])"),QString::SkipEmptyParts);
          qDebug() << "process state: " << s.join(" ").toLower();

      });

      emit canMeasure();
    }
    else
        qDebug() << "failed to configure process";
}

void CognitiveTestManager::readOutput()
{
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
    for(auto&& x : dir.entryList())
    {
        if (x.endsWith(".csv"))
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
        qDebug() << "found output csv file " << fileName;
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

void CognitiveTestManager::setInputs(const QMap<QString,QVariant> &inputs)
{
    // TODO: check minimum inputs required
    //
    m_inputData = inputs;
    configureProcess();
}

void  CognitiveTestManager::measure()
{
   // launch the process
    qDebug() << "starting process from measure";
    m_process.start();
    //m_process.waitForFinished();
}

void  CognitiveTestManager::clean()
{
    if(!m_outputFile.isEmpty())
    {
        QFile ofile(m_outputFile);
        ofile.remove();
    }
}

QJsonObject CognitiveTestManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    QFile ofile(m_outputFile);
    ofile.open(QIODevice::ReadOnly);
    QByteArray buffer = ofile.readAll();
    json.insert("test_csv_file",QString(buffer.toBase64()));
    return json;
}
