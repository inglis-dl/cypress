#include "TonometerManager.h"

#include "../data/AccessQueryHelper.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>
#include <QSqlDatabase>
//#include <QSqlQuery>
#include <QStandardItemModel>

TonometerManager::TonometerManager(QObject* parent):
    ManagerBase(parent)
{
    setGroup("tonometer");

    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode"; // ID column in Patients table
    m_inputKeyList << "language";

    // tonometer specific mandator inputs
    //
    m_inputKeyList << "date_of_birth"; // format dd/MM/YY 00:00:00, BirthDate column in Patients table
    m_inputKeyList << "sex";           // 0  = female, 1 = male, Sex column in Patients table
}

void TonometerManager::start()
{
    // connect signals and slots to QProcess one time only
    //
    connect(&m_process, &QProcess::started,
        this, [this]() {
            qDebug() << "process started: " << m_process.arguments().join(" ");
        });

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &TonometerManager::readOutput);

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

    configureProcess();
    emit dataChanged();
}

void TonometerManager::buildModel(QStandardItemModel *model) const
{
    QVector<QString> v_side({"left","right"});
    for(auto&& side : v_side)
    {
      int col = "left" == side ? 0 : 1;
      QStringList list = m_test.getMeasurementStrings(side);
      for(int row=0;row<list.size();row++)
      {
        TonometerMeasurement m; // = m_test.getMeasurement(side,row);
        QStandardItem* item = model->item(row,col);
        item->setData(list.at(row), Qt::DisplayRole);
      }
    }
}

void TonometerManager::loadSettings(const QSettings& settings)
{
    // the full spec path name including exe name
    // eg., C:\Program Files (x86)\Reichert\ORA.exe
    //
    QString exeName = settings.value(getGroup() + "/client/exe").toString();
    selectRunnable(exeName);
    QString dbName = settings.value(getGroup() + "/client/mdb").toString();
    selectDatabase(dbName);
}

void TonometerManager::saveSettings(QSettings* settings) const
{
    if (!m_runnableName.isEmpty())
    {
        settings->beginGroup(getGroup());
        settings->setValue("client/exe", m_runnableName);
        settings->setValue("client/mdb", m_databaseName);
        settings->endGroup();
        if (m_verbose)
            qDebug() << "wrote exe fullspec path to settings file";
    }
}

QJsonObject TonometerManager::toJsonObject() const
{
    QJsonObject json;
    if(m_test.isValid())
      json = m_test.toJsonObject();
    return json;
}

bool TonometerManager::isDefined(const QString &fileName) const
{
    if("simulate" == m_mode)
    {
       return true;
    }
    return (!fileName.isEmpty() &&  QFileInfo::exists(fileName));
}

void TonometerManager::selectRunnable(const QString &exeName)
{
    if(isDefined(exeName))
    {
       QFileInfo info(exeName);
       m_runnableName = exeName;
       m_runnablePath = info.absolutePath();
       emit runnableSelected();
       configureProcess();
    }
    else
       emit canSelectRunnable();
}

void TonometerManager::selectDatabase(const QString &dbName)
{
    if(isDefined(dbName))
    {
       m_databaseName = dbName;
       m_temporaryFile = m_databaseName + ".ORIG";
       emit databaseSelected();
       configureProcess();
    }
    else
       emit canSelectDatabase();
}

void TonometerManager::measure()
{
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

void TonometerManager::setInputData(const QMap<QString, QVariant> &input)
{
    if("simulate" == m_mode)
    {
        m_inputData["barcode"] = "00000000";
        m_inputData["language"] = "english";
        m_inputData["date_of_birth"] = "1965-12-17";
        m_inputData["sex"] = -1;
        return;
    }
    bool ok = true;
    m_inputData = input;
    for(auto&& x : m_inputKeyList)
    {
        if(m_inputData.contains(x))
        {
           QVariant value = m_inputData[x];
           if("sex" == x)
           {
             if(QVariant::Type::String == value.type())
             {
                m_inputData[x] = value.toString().toLower().startsWith("f") ? 0 : -1;
             }
             // TODO: handle DOB input formatting
             //
           }
        }
        else
        {
            ok = false;
            qDebug() << "ERROR: missing expected input " << x;
            break;
        }
    }
    if(!ok)
        m_inputData.clear();
    else
        configureProcess();
}

void TonometerManager::readOutput()
{
    if("simulate" == m_mode)
    {
        qDebug() << "simulating read out";

        //TODO: impl left and right measurements
        //
        TonometerMeasurement m;
        m.setCharacteristic("name","IOPG");
        m.setCharacteristic("value", 1.0);
        m.setCharacteristic("units","mmHg");
        m_test.addMeasurement(m);
        m.setCharacteristic("name","IOPCC");
        m.setCharacteristic("value", 1.0);
        m.setCharacteristic("units","mmHg");
        m_test.addMeasurement(m);
        m.setCharacteristic("name","CH");
        m.setCharacteristic("value", 1.0);
        m.setCharacteristic("units","mmHg");
        m_test.addMeasurement(m);
        m.setCharacteristic("name","CRF");
        m.setCharacteristic("value", 1.0);
        m.setCharacteristic("units","mmHg");
        m_test.addMeasurement(m);

        for(auto&& x : m_inputData.toStdMap())
        {
          m_test.addMetaDataCharacteristic(x.first,x.second);
        }

        // emit the can write signal
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

    if(isDefined(m_databaseName))
    {
      AccessQueryHelper helper;
      helper.setOperation(AccessQueryHelper::Operation::Results);
      QVariant result = helper.processQuery(m_inputData,m_db);
      if(result.isValid() && !result.isNull())
      {
        QJsonArray arr = QJsonValue::fromVariant(result).toArray();
        // if there are multiple session dates, we only want the most recent one
        m_test.fromJson(arr);
        if(m_test.isValid())
        {
          emit message(tr("Ready to save results..."));
          emit canWrite();
        }
        else
          qDebug() << "ERROR: ora database produced invalid test results";
      }
      emit dataChanged();
    }
    else
      qDebug() << "ERROR: ora database is missing";
}

void TonometerManager::configureProcess()
{
    if("simulate" == m_mode)
    {
        emit message(tr("Ready to measure..."));
        emit canMeasure();
        return;
    }
    // ORA.exe, ora.mdb and input file are present
    //
    QDir working(m_runnablePath);
    if(isDefined(m_runnableName) &&
        isDefined(m_databaseName) &&
        working.exists())
    {
        qDebug() << "OK: configuring command";

        m_process.setProcessChannelMode(QProcess::ForwardedChannels);
        m_process.setProgram(m_runnableName);
        m_process.setWorkingDirectory(m_runnablePath);

        qDebug() << "process working dir: " << m_runnablePath;

        // backup the original ora.mdb
        if(QFileInfo::exists(m_temporaryFile))
        {
            QFile tfile(m_temporaryFile);
            tfile.remove();
        }
        QFile::copy(m_databaseName, m_temporaryFile);
        qDebug() << "wrote backup to " << m_temporaryFile;

        if(!QSqlDatabase::contains("mdb_connection"))
        {
          m_db = QSqlDatabase::addDatabase("QODBC", "mdb_connection");
          m_db.setDatabaseName(
            "DRIVER={Microsoft Access Driver (*.mdb, ^.accdb)};FIL={MS Access};DBQ=" + m_databaseName);
        }
        if(m_db.open())
        {
            bool insert = true;
            AccessQueryHelper helper;
            helper.setOperation(AccessQueryHelper::Operation::Count);
            QVariant result = helper.processQuery(m_inputData,m_db);
            // first check if the query failed
            if((result.canConvert(QMetaType::Bool) && result.toBool()) ||
               (result.canConvert(QMetaType::Int) && -1 == result.toInt()))
            {
                qDebug() << "ERROR: configuration failed during count query";
                insert = false;
            }
            else
            {
              if(0 < result.toInt()) // clear out participant data
              {
                helper.setOperation(AccessQueryHelper::Operation::Delete);
                result = helper.processQuery(m_inputData,m_db);
                if(!result.toBool())
                {
                  qDebug() << "ERROR: configuration failed during delete query";
                  insert = false;
                }
              }
            }
            if(insert)
            {
                helper.setOperation(AccessQueryHelper::Operation::Insert);
                result = helper.processQuery(m_inputData,m_db);
                if(result.toBool())
                {
                  emit message(tr("Ready to measure..."));
                  emit canMeasure();
                }
                else
                  qDebug() << "ERROR: configuration failed during insert query";
            }
        }
    }
    else
      qDebug() << "failed to configure process";
}

void TonometerManager::clearData()
{
    m_test.reset();
    emit dataChanged();
}

void TonometerManager::finish()
{
    m_test.reset();
    if("simulate" == m_mode)
    {
      return;
    }
    AccessQueryHelper helper;
    helper.setOperation(AccessQueryHelper::Operation::Delete);
    QVariant result = helper.processQuery(m_inputData,m_db);
    if(!result.toBool())
    {
      qDebug() << "ERROR: finish failed during delete query";
    }

    if(m_db.isOpen())
    {
      m_db.close();
    }

    if(QProcess::NotRunning != m_process.state())
    {
      m_process.close();
    }

    if(!m_temporaryFile.isEmpty() && QFileInfo::exists(m_temporaryFile))
    {
        // remove the inputfile first
        QFile ifile(m_databaseName);
        ifile.remove();
        QFile::copy(m_temporaryFile, m_databaseName);
        qDebug() << "restored backup from " << m_temporaryFile;
        QFile tempFile(m_temporaryFile);
        tempFile.remove();
        m_temporaryFile.clear();
    }

    QSqlDatabase::removeDatabase("mdb_connection");
}
