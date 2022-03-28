#include "CDTTManager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>
#include <QSqlDatabase>
#include <QStandardItemModel>

CDTTManager::CDTTManager(QObject* parent) : ManagerBase(parent)
{
    setGroup("cdtt");
    m_col = 1;
    m_row = 8;

    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode";
    m_inputKeyList << "language";
}

CDTTManager::~CDTTManager()
{
  QSqlDatabase::removeDatabase("xlsx_connection");
}

void CDTTManager::start()
{
    // connect signals and slots to QProcess one time only
    //
    connect(&m_process, &QProcess::started,
        this, [this]() {
            qDebug() << "process started: " << m_process.arguments().join(" ");
        });

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &CDTTManager::readOutput);

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

void CDTTManager::loadSettings(const QSettings& settings)
{
    // the full spec path name including jar name
    // eg., C:\Users\clsa\Documents\CDTT-2018-07-22\CDTTstereo.jar
    //
    QString jarName = settings.value(getGroup() + "/client/jar").toString();
    selectRunnable(jarName);
}

void CDTTManager::saveSettings(QSettings* settings) const
{
    if(!m_runnableName.isEmpty())
    {
        settings->beginGroup(getGroup());
        settings->setValue("client/jar", m_runnableName);
        settings->endGroup();
        if (m_verbose)
            qDebug() << "wrote jar fullspec path to settings file";
    }
}

QJsonObject CDTTManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    if(Constants::RunMode::modeSimulate != m_mode)
    {
        QFile ofile(m_outputFile);
        ofile.open(QIODevice::ReadOnly);
        QByteArray buffer = ofile.readAll();
        json.insert("test_output_file", QString(buffer.toBase64()));
        json.insert("test_output_file_mime_type", "xlsx");
    }
    json.insert("test_input",m_inputData);
    return json;
}

void CDTTManager::buildModel(QStandardItemModel* model) const
{
    int row = 0;
    foreach(const auto str, m_test.toStringList())
    {
        QStandardItem* item = model->item(row, 0);
        if(Q_NULLPTR == item)
        {
            item = new QStandardItem();
            model->setItem(row, 0, item);
        }
        item->setData(str, Qt::DisplayRole);
        row++;
    }
}

bool CDTTManager::isDefined(const QString& runnableName) const
{
    bool ok = false;
    if(!runnableName.isEmpty())
    {
        QFileInfo info(runnableName);
        if(info.exists() && "jar" == info.completeSuffix())
        {
            QString path = info.absolutePath();

            qDebug() << "path to " << runnableName << " is " << path;
            QDir dir = QDir::cleanPath(path + QDir::separator() + "applicationFiles" + QDir::separator() + "Results");
            if (dir.exists())
            {
                ok = true;
                qDebug() << "OK: results directory exists " << dir.absolutePath();
            }
            else
                qDebug() << "ERROR: results directory not found " << dir.path();
        }
        else
            qDebug() << "ERROR: info does not exist for file " << runnableName;
    }
    else
        qDebug() << "ERROR: isDefined check on empty string";
    return ok;
}

void CDTTManager::selectRunnable(const QString &runnableName)
{
    if(isDefined(runnableName))
    {
        QFileInfo info(runnableName);
        m_runnableName = runnableName;
        m_runnablePath = info.absolutePath();
        QDir dir = QDir::cleanPath(m_runnablePath + QDir::separator() + "applicationFiles" + QDir::separator() + "Results");
        m_outputPath = dir.path();

        emit runnableSelected();
        configureProcess();
    }
    else
        emit canSelectRunnable();
}

void CDTTManager::measure()
{
    clearData();
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        readOutput();
        return;
    }
    // launch the process
    qDebug() << "starting process from measure";
    m_process.start();
}

void CDTTManager::setInputData(const QJsonObject& input)
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
    if(!ok)
    {
      if(m_verbose)
        qDebug() << "ERROR: invalid input data";

      emit message(tr("ERROR: the input data is incorrect"));
      m_inputData= QJsonObject();
    }
    else
      configureProcess();
}

void CDTTManager::readOutput()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        m_test.simulate(m_inputData["barcode"].toString());
        if(m_test.isValid())
        {
          emit message(tr("Ready to save results..."));
          emit canWrite();
        }
        else
            qDebug() << "invalid test results";
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

    QDir dir(m_outputPath);
    QString fileName = dir.filePath(QString("Results-%0.xlsx").arg(getInputDataValue("barcode").toString()));
    if(QFileInfo::exists(fileName))
    {
      qDebug() << "found output xlsx file " << fileName;

      //TODO: impl for linux or insert ifdef OS blockers
      //
      QSqlDatabase db;
      if(!QSqlDatabase::contains("xlsx_connection"))
      {
        db = QSqlDatabase::addDatabase("QODBC", "xlsx_connection");
        db.setDatabaseName(
          "DRIVER={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};DBQ=" + fileName);
        if(db.isValid())
          db.open();
        else
          qDebug() << "ERROR: invalid database using" << fileName;
      }
      else
        db = QSqlDatabase::database("xlsx_connection");

      if(db.isValid() && !db.isOpen())
          db.open();
      if(db.isOpen())
      {
        m_test.fromDatabase(db);
        m_outputFile.clear();
        if(m_test.isValid())
        {
          emit message(tr("Ready to save results..."));
          emit canWrite();
          m_outputFile = fileName;
        }
        else
          qDebug() << "ERROR: input from file produced invalid test results";

        db.close();
      }
      emit dataChanged();
    }
    else
        qDebug() << "ERROR: no output xlsx file found"<<fileName;
}

void CDTTManager::clearData()
{
    m_test.reset();
    emit dataChanged();
}

void CDTTManager::finish()
{
    m_test.reset();
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        return;
    }
    if(QProcess::NotRunning != m_process.state())
    {
        m_process.kill();
    }
    if(!m_outputFile.isEmpty() && QFileInfo::exists(m_outputFile))
    {
        QFile ofile(m_outputFile);
        ofile.remove();
        m_outputFile.clear();
    }
}

void CDTTManager::configureProcess()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        if(!m_inputData.isEmpty())
        {
          emit message(tr("Ready to measure..."));
          emit canMeasure();
        }
        return;
    }

    QDir working(m_runnablePath);
    QDir out(m_outputPath);
    if(isDefined(m_runnableName) &&
       working.exists() && out.exists() &&
       !m_inputData.isEmpty())
    {
        qDebug() << "OK: configuring command";

        // TODO: Check that a correct version of java is installed
        QString command = "java";
        QStringList arguments;
        arguments << "-jar"
            << m_runnableName
            << getInputDataValue("barcode").toString();

        m_process.setProgram(command);
        m_process.setArguments(arguments);
        m_process.setWorkingDirectory(working.absolutePath());

        qDebug() << "process config args: " << m_process.arguments().join(" ");
        qDebug() << "process working dir: " << working.absolutePath();

        emit message(tr("Ready to measure..."));
        emit canMeasure();
    }
    else
        qDebug() << "failed to configure process";
}
