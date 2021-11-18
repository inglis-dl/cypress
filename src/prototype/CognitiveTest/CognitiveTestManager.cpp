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

void CognitiveTestManager::loadSettings(const QSettings &settings)
{
    // the full spec path name including exe name
    // eg., C:\Program Files (x86)\CCB\CCB.exe
    //
    QString s = settings.value("client/exe").toString();
    if(!s.isEmpty())
    {
        QFileInfo info(s);
        if(info.exists() && info.isExecutable())
        {
            m_executableName = info.fileName();
            m_executablePath = info.filePath();
            QDir dir = QDir::cleanPath(m_executablePath + QDir::separator() + "results");
            if(dir.exists())
            {
                m_outputPath = dir.path();
            }
            else
            {
                m_executableName.clear();
                m_executablePath.clear();
                m_outputPath.clear();
            }
        }
    }
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
    emit dataChanged();
}

void CognitiveTestManager::configureProcess()
{
    // the exe is present
    QFileInfo info(m_executableName);
    QDir working(m_executablePath);
    QDir out(m_outputPath);
    if((info.exists() && (info.isExecutable() || info.isFile())) &&
        working.exists() && out.exists())
    {
      // the inputs for command line args are present
      QStringList command;
      command << m_executableName;

      if(m_inputData.contains("barcode"))
         command << "/i" + m_inputData["barcode"].toString();

      if(m_inputData.contains("site"))
         command << "/c" + m_inputData["site"].toString();

      if(m_inputData.contains("user"))
         command << "/u" + m_inputData["site"].toString();

      if(m_inputData.contains("language"))
         command << "/l" + m_inputData["language"].toString();

      m_process.setArguments(command);
      m_process.setWorkingDirectory(m_executablePath);

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

      emit canMeasure();
    }
}

void CognitiveTestManager::readOutput()
{
    if(QProcess::NormalExit != m_process.exitStatus())
    {
        qDebug() << "ERROR: process failed to finish correctly: cannot read output";
        return;
    }
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
        m_test.fromFile(fileName);
        if(m_test.isValid())
        {
            emit dataChanged();
            emit canWrite();
        }
    }
}

void CognitiveTestManager::setInputs(const QMap<QString,QVariant> &inputs)
{
    m_inputData = inputs;
    configureProcess();
}

void  CognitiveTestManager::measure()
{
   // launch the process
    m_process.start();
}

QJsonObject CognitiveTestManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    return json;
}
