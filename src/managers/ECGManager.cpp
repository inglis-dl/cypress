#include "ECGManager.h"

#include "../auxiliary/Utilities.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QJsonObject>
#include <QSettings>
#include <QStandardItemModel>

ECGManager::ECGManager(QObject* parent):
    ManagerBase(parent)
{
    setGroup("ecg");
    m_col = 1;
    m_row = 19;

    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode";
    m_inputKeyList << "language";
}

void ECGManager::start()
{
    // connect signals and slots to QProcess one time only
    //
    connect(&m_process, &QProcess::started,
        this, [this]() {
            qDebug() << "process started: " << m_process.arguments().join(" ");
        });

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &ECGManager::readOutput);

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

void ECGManager::buildModel(QStandardItemModel *model) const
{
    // add the measurement output
    //
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

void ECGManager::loadSettings(const QSettings& settings)
{
    // the full spec path name including exe name
    //
    QString exeName = settings.value(getGroup() + "/client/exe").toString();
    selectRunnable(exeName);
    QString workingName = settings.value(getGroup() + "/client/working").toString();
    selectWorking(workingName);

}

void ECGManager::saveSettings(QSettings* settings) const
{
    if(!m_runnableName.isEmpty())
    {
        settings->beginGroup(getGroup());
        settings->setValue("client/exe", m_runnableName);
        settings->setValue("client/working", m_workingPath);
        settings->endGroup();
        if(m_verbose)
          qDebug() << "wrote exe fullspec path to settings file";
    }
}

QJsonObject ECGManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    if(Constants::RunMode::modeSimulate != m_mode)
    {
      QFile ofile(m_outputFile);
      ofile.open(QIODevice::ReadOnly);
      QByteArray buffer = ofile.readAll();
      json.insert("test_output_file",QString(buffer.toBase64()));
      json.insert("test_output_file_mime_type","xml");
    }
    json.insert("test_input",m_inputData);
    return json;
}

bool ECGManager::isDefined(const QString &fileName, const FileType &type) const
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
       return true;
    }
    bool ok = false;
    if(type == FileType::ECGApplication)
    {
        QFileInfo info(fileName);
        ok = info.exists() && info.isExecutable();
    }
    else
    {
        QDir info(fileName);
        ok = info.exists() && info.isReadable() && info.exists("Export");
    }
    return ok;
}

void ECGManager::selectRunnable(const QString &runnableName)
{
    if(isDefined(runnableName))
    {
        m_runnableName = runnableName;
        qDebug() << "set runnable" << m_runnableName;
        emit runnableSelected();
        configureProcess();
    }
    else
        emit canSelectRunnable();
}

void ECGManager::selectWorking(const QString& workingName)
{
    if(isDefined(workingName,ECGManager::FileType::ECGWorkingDir))
    {
        m_workingPath = workingName;
        m_exportPath = QDir(m_workingPath).filePath("Export");
        qDebug() << "set working path" << m_workingPath;
        qDebug() << "set export path" << m_exportPath;
        emit workingSelected();
        configureProcess();
    }
    else
        emit canSelectWorking();
}

void ECGManager::select()
{
    QFileDialog dialog;
    // which do we need to select first ?
    QString caption;
    QStringList filters;
    bool selectingRunnable = false;
    if(!isDefined(m_runnableName, FileType::ECGApplication))
    {
       filters << "Applications (*.exe)" << "Any files (*)";
       caption = tr("Select ora.exe File");
       selectingRunnable = true;
       dialog.setNameFilters(filters);
       dialog.setFileMode(QFileDialog::ExistingFile);
    }
    else if(!isDefined(m_workingPath, FileType::ECGWorkingDir))
    {
       dialog.setFileMode(QFileDialog::FileMode::DirectoryOnly);
       caption = tr("Select path to Cardiosoft export directory");
    }
    else
      return;

    dialog.setWindowTitle(caption);
    if(dialog.exec() == QDialog::Accepted)
    {
      QStringList files = dialog.selectedFiles();
      QString fileName = files.first();
      FileType type =
        (selectingRunnable ? FileType::ECGApplication : FileType::ECGWorkingDir);
      if(isDefined(fileName,type))
      {
        if(selectingRunnable)
        {
          selectRunnable(fileName);
        }
        else
        {
          selectWorking(fileName);
        }
      }
   }
}

void ECGManager::measure()
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

void ECGManager::setInputData(const QJsonObject &input)
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
      else
      {
        const QVariant value = m_inputData[key].toVariant();
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
    }
    if(!ok)
    {
      if(m_verbose)
        qDebug() << "ERROR: invalid input data";

      emit message(tr("ERROR: the input data is incorrect"));
      m_inputData = QJsonObject();
    }
    else
      configureProcess();
}

void ECGManager::readOutput()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        if(m_verbose)
          qDebug() << "simulating read out";

        m_test.simulate();
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
      qDebug() << "process finished successfully";

    QString xmlFile = QString("%1.xml").arg(m_inputData["barcode"].toString());
    QDir exportDir(m_exportPath);
    m_outputFile = exportDir.filePath(xmlFile);
    if(QFileInfo::exists(m_outputFile))
    {
        qDebug() << "found xml output file " << m_outputFile;
        m_test.fromFile(m_outputFile);
        if(m_test.isValid())
        {
          emit message(tr("Ready to save results..."));
          emit canWrite();
        }
        else
          qDebug() << "ERROR: input from file produced invalid test results";

        emit dataChanged();
    }
    else
      qDebug() << "ERROR: no output xml file found";
}

void ECGManager::configureProcess()
{
    if(Constants::RunMode::modeSimulate == m_mode &&
       !m_inputData.isEmpty())
    {
        emit message(tr("Ready to measure..."));
        emit canMeasure();
        return;
    }

    QDir workingDir(m_workingPath);
    QDir exportDir(m_exportPath);
    if(isDefined(m_runnableName) &&
       workingDir.exists() && exportDir.exists() &&
       !m_inputData.isEmpty())
    {
        qDebug() << "OK: configuring command";

        m_process.setProgram(m_runnableName);
       // m_process.setWorkingDirectory(m_runnablePath); // just use the default

        QString path = QDir::cleanPath(QString("%1%2%3").arg(m_workingPath,QDir::separator(),INIT_PATH));
        QDir backupDir(path);
        if(backupDir.exists())
        {
          if(!backupDir.removeRecursively())
              qDebug() << "failed to configure and remove backup directory"<<path;
        }

        if(deleteDeviceData())
        {
          emit message(tr("Ready to measure..."));
          emit canMeasure();
        }
    }
    else
        qDebug() << "failed to configure process";
}

void ECGManager::clearData()
{
    m_test.reset();
    emit dataChanged();
}

void ECGManager::finish()
{
    m_test.reset();
    if(QProcess::NotRunning != m_process.state())
    {
        m_process.close();
    }

    deleteDeviceData();
}

bool ECGManager::deleteDeviceData()
{
    bool ok = true;
    QString xmlFile = QString("%1.xml").arg(m_inputData["barcode"].toString());
    QString path = QDir::cleanPath(QString("%1%2%3").arg(m_workingPath,QDir::separator(),INIT_PATH));
    QDir backupDir(path);
    if(!backupDir.exists())
    {
        if(!backupDir.mkdir(path))
        {
            qCritical() << "unable to create backup directory" << path;
            ok = false;
            return ok;
        }
    }
    if(ok)
    {
      // list of backed up database files
      //
      backupDir.setNameFilters(QStringList()<<"*.BTR");
      backupDir.setFilter(QDir::Files);
      QFileInfoList list = backupDir.entryInfoList();
      if(!list.isEmpty())
      {
        path = QDir::cleanPath(QString("%1%2%3").arg(m_workingPath,QDir::separator(),DATABASE_PATH));
        foreach(const auto info, list)
        {
            QString toFile = QDir(path).filePath(info.fileName());
            QFile::copy(info.absoluteFilePath(), toFile);
        }
      }
    }

    // xml output from Cardiosoft export
    //
    QString outfileName = QDir(m_exportPath).filePath(xmlFile);
    QFile file(outfileName);
    if(file.exists() && !file.remove())
    {
        qDebug() << "could not delete Cardiosoft xml output file" << outfileName;
        ok = false;
    }
    return ok;
}
