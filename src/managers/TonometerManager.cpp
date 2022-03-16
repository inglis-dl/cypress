#include "TonometerManager.h"

#include "../data/AccessQueryHelper.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QStandardItemModel>

TonometerManager::TonometerManager(QObject* parent):
    ManagerBase(parent)
{
    setGroup("tonometer");
    m_col = 2;
    m_row = 8;

    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode"; // ID column in Patients table
    m_inputKeyList << "language";

    // tonometer specific mandator inputs
    //
    m_inputKeyList << "date_of_birth"; // format dd/MM/YY 00:00:00, BirthDate column in Patients table
    m_inputKeyList << "sex";           // 0  = female, 1 = male, Sex column in Patients table

    m_test.setMaximumNumberOfMeasurements(2);
}

TonometerManager::~TonometerManager()
{
  QSqlDatabase::removeDatabase("mdb_connection");
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
    foreach(auto side, v_side)
    {
      int col = "left" == side ? 0 : 1;
      QStringList list = m_test.getMeasurementStrings(side);
      for(int row = 0; row < list.size(); row++)
      {
        QStandardItem* item = model->item(row,col);
        if(Q_NULLPTR == item)
        {
            item = new QStandardItem();
            model->setItem(row,col,item);
        }
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
    QJsonObject json = m_test.toJsonObject();
    json.insert("test_input",m_inputData);
    return json;
}

bool TonometerManager::isDefined(const QString &fileName, TonometerManager::FileType type) const
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
       return true;
    }
    bool ok = false;
    QFileInfo info(fileName);
    if(type == TonometerManager::FileType::ORAApplication)
    {
        ok = info.isExecutable() && info.exists();
    }
    else
    {
        ok = info.isFile() && info.exists();
    }
    return ok;
}

void TonometerManager::select()
{
    // which do we need to select first ?
    QString caption;
    QStringList filters;
    bool selectingRunnable = false;
    if(!isDefined(m_runnableName, TonometerManager::FileType::ORAApplication))
    {
       filters << "Applications (*.exe)" << "Any files (*)";
       caption = tr("Select ora.exe File");
       selectingRunnable = true;
    }
    else if(!isDefined(m_databaseName, TonometerManager::FileType::ORADatabase))
    {
       filters << "MS Access (*.mdb)" << "Any files (*)";
       caption = tr("Select ora.mdb File");
    }
    else
      return;

    QFileDialog dialog;
    dialog.setNameFilters(filters);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setWindowTitle(caption);
    if(dialog.exec() == QDialog::Accepted)
    {
      QStringList files = dialog.selectedFiles();
      QString fileName = files.first();
      FileType type =
        (selectingRunnable ? FileType::ORAApplication : FileType::ORADatabase);
      if(isDefined(fileName,type))
      {
        if(selectingRunnable)
        {
          selectRunnable(fileName);
        }
        else
        {
          selectDatabase(fileName);
        }
      }
   }
}

void TonometerManager::selectRunnable(const QString &exeName)
{
    if(isDefined(exeName,FileType::ORAApplication))
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
    if(isDefined(dbName,FileType::ORADatabase))
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
    if(Constants::RunMode::modeSimulate == m_mode)
    {
      readOutput();
      return;
    }
    clearData();
    // launch the process
    if(m_verbose)
      qDebug() << "Starting process from measure";

    m_process.start();
}

void TonometerManager::setInputData(const QJsonObject &input)
{
    m_inputData = input;
    if(Constants::RunMode::modeSimulate == m_mode)
    {
      if(!input.contains("barcode"))
        m_inputData["barcode"] = Constants::DefaultBarcode;
      if(!input.contains("language"))
          m_inputData["language"] = "en";
      if(!input.contains("date_of_birth"))
        m_inputData["date_of_birth"] =
          QJsonValue::fromVariant(QVariant(QDate::fromString("1965-12-17","yyyy-MM-dd")));
      if(!input.contains("sex"))
        m_inputData["sex"] = "male";
    }
    bool ok = true;
    QMap<QString,QMetaType::Type> typeMap {
        {"barcode",QMetaType::Type::QString},
        {"language",QMetaType::Type::QString},
        {"sex",QMetaType::Type::QString},
        {"date_of_birth",QMetaType::Type::QDate }
    };
    foreach(auto key, m_inputKeyList)
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
}

void TonometerManager::readOutput()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        m_test.simulate(m_inputData);
        if(m_test.isValid())
        {
          // emit the can write signal
          emit message(tr("Ready to save results..."));
          emit canWrite();
        }
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

    QSqlDatabase db = QSqlDatabase::database("mdb_connection");
    if(db.isValid() && !db.isOpen())
        db.open();
    if(db.isOpen())
    {
      // TODO: check if QJsonObject can convert variant back to date
      //
      QMap<QString,QVariant> input;
      input.insert("barcode",m_inputData["barcode"].toString());
      QVariant value = m_inputData["sex"].toString().toLower().startsWith("f") ? 0 : -1;
      input.insert("sex",value);
      input.insert("date_of_birth",m_inputData["date_of_birth"].toVariant());
      AccessQueryHelper helper;
      helper.setOperation(AccessQueryHelper::Operation::Results);
      QVariant result = helper.processQuery(input,db);
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
      db.close();
    }
    else
      qDebug() << "ERROR: ora database is missing";
}

void TonometerManager::configureProcess()
{
    if(m_inputData.isEmpty()) return;

    if(Constants::RunMode::modeSimulate == m_mode)
    {
        emit message(tr("Ready to measure..."));
        emit canMeasure();
        return;
    }

    // ORA.exe, ora.mdb and input file are present
    //
    QDir working(m_runnablePath);
    if(isDefined(m_runnableName, FileType::ORAApplication) &&
       isDefined(m_databaseName, FileType::ORADatabase) &&
       working.exists())
    {
        qDebug() << "OK: configuring command";

        m_process.setProcessChannelMode(QProcess::ForwardedChannels);
        m_process.setProgram(m_runnableName);
        m_process.setWorkingDirectory(m_runnablePath);

        qDebug() << "process working dir: " << m_runnablePath;

        // backup the original ora.mdb
        if(!QFileInfo::exists(m_temporaryFile))
        {
            QFile::copy(m_databaseName, m_temporaryFile);
            qDebug() << "wrote backup to " << m_temporaryFile;
        }

        QSqlDatabase db;
        if(!QSqlDatabase::contains("mdb_connection"))
        {
          db = QSqlDatabase::addDatabase("QODBC", "mdb_connection");
          db.setDatabaseName(
            "DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};FIL={MS Access};DBQ=" + m_databaseName);
          if(db.isValid())
            db.open();
          else
            qDebug() << "ERROR: invalid database using"<<m_databaseName;
        }
        else
          db = QSqlDatabase::database("mdb_connection");

        if(db.isValid() && !db.isOpen())
            db.open();
        if(db.isOpen())
        {
            // TODO:
            // for testing on Win with no actual device connected to ORA
            // - create ora.mdb with testing barcode in Patients table
            //   and linkage to Measures table
            // - launch ora software and close with no changes to ora.mdb
            // - requires skipping the next db actions to remove any pre-existing
            //   participant records
            //

            // case 1) - db has no particpant records in the Patients or Measures table
            // - insert one particpant record to the db
            //
            // case 2) - db has records in both the Measures and Patients tables
            // - delete all Meausures records
            // - delete all Patients records
            // - insert one particpant record to the db
            //
            // case 3) - db has 1 record in the Patients table, no records in the Measures table
            // - no insert required
            //

            // require sex and date_of_birth
            //
            QMap<QString,QVariant> input;
            input.insert("barcode",m_inputData["barcode"].toString());
            QVariant value = m_inputData["sex"].toString().toLower().startsWith("f") ? 0 : -1;
            input.insert("sex",value);
            input.insert("date_of_birth",m_inputData["date_of_birth"].toVariant());

            bool insert = true;
            AccessQueryHelper helper;
            helper.setOperation(AccessQueryHelper::Operation::CountMeasures);
            QVariant result = helper.processQuery(input,db);
            // first check if the query failed
            if(-1 == result.toInt())
            {
              qDebug() << "ERROR: configuration failed count query";
              insert = false;
            }
            else if(0 < result.toInt())
            {
              // clear out participant data from Measures
              helper.setOperation(AccessQueryHelper::Operation::DeleteMeasures);
              result = helper.processQuery(input,db);
              if(!result.toBool())
              {
                qDebug() << "ERROR: configuration failed delete query";
                insert = false;
              }
            }

            helper.setOperation(AccessQueryHelper::Operation::Count);
            result = helper.processQuery(input,db);
            // first check if the query failed
            if(-1 == result.toInt())
            {
              qDebug() << "ERROR: configuration failed count query";
              insert = false;
            }
            else if(1 < result.toInt())
            {
              // clear out participant data from Patients
              helper.setOperation(AccessQueryHelper::Operation::Delete);
              result = helper.processQuery(input,db);
              if(!result.toBool())
              {
                qDebug() << "ERROR: configuration failed delete query";
                insert = false;
              }
            }
            else if(1 == result.toInt())
            {
              insert = false;
            }

            if(insert)
            {
              helper.setOperation(AccessQueryHelper::Operation::Insert);
              result = helper.processQuery(input,db);
              if(result.toBool())
              {
                // verify we have only 1 entry in the Patients table
                helper.setOperation(AccessQueryHelper::Operation::Count);
                result = helper.processQuery(input,db);
                if(1 != result.toInt())
                {
                  qDebug() << "ERROR: configuration failed insert non-unary"
                           << QString::number(result.toInt());
                }
                else
                {
                  emit message(tr("Ready to measure..."));
                  emit canMeasure();
                }
              }
              else
                qDebug() << "ERROR: configuration failed during insert query";
            }
          db.close();
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
    if(Constants::RunMode::modeSimulate == m_mode)
    {
      return;
    }
    QSqlDatabase db = QSqlDatabase::database("mdb_connection");
    if(db.isValid() && !db.isOpen())
        db.open();
    if(db.isOpen())
    {
      AccessQueryHelper helper;
      helper.setOperation(AccessQueryHelper::Operation::DeleteMeasures);

      QMap<QString,QVariant> input;
      input.insert("barcode",m_inputData["barcode"].toString());
      QVariant value = m_inputData["sex"].toString().toLower().startsWith("f") ? 0 : -1;
      input.insert("sex",value);
      input.insert("date_of_birth",m_inputData["date_of_birth"].toVariant());

      QVariant result = helper.processQuery(input,db);
      if(!result.toBool())
      {
        qDebug() << "ERROR: finish failed during delete measures query";
      }
      helper.setOperation(AccessQueryHelper::Operation::Delete);
      result = helper.processQuery(input,db);
      if(!result.toBool())
      {
        qDebug() << "ERROR: finish failed during delete patient query";
      }
      db.close();
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
}
