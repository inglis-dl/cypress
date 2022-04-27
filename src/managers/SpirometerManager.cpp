#include "SpirometerManager.h"

#include <QFileDialog>
#include <QStandardItemModel>

#include "../auxiliary/JsonSettings.h"
#include "EMRPluginWriter.h"

SpirometerManager::SpirometerManager(QObject* parent) : ManagerBase(parent)
{
    setGroup("spirometer");

    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode";
    m_inputKeyList << "language";
    m_inputKeyList << "gender";  // male/female
    m_inputKeyList << "date_of_birth";
    m_inputKeyList << "height";  // m
    m_inputKeyList << "weight";  // kg
    m_inputKeyList << "smoker";  // true/false

    //TODO: check if these upstream inputs are optinal or required
    //
    //m_inputKeyList << "asthma"; // true/false
    //m_inputKeyList << "copd";   // true/false
    //m_inputKeyList << "ethnicity"; // caucasian, asian, african, hispanic, other_ethnic

    m_test.setExpectedMeasurementCount(4);
}

void SpirometerManager::initializeModel()
{
    for(int col = 0; col < 4; col++)
    {
        for(int row = 0; row < 8; row++)
        {
            QStandardItem* item = new QStandardItem();
            m_model->setItem(row, col, item);
        }
    }
    m_model->setHeaderData(0, Qt::Horizontal, "Variables", Qt::DisplayRole);
    m_model->setHeaderData(1, Qt::Horizontal, "Trial 1", Qt::DisplayRole);
    m_model->setHeaderData(2, Qt::Horizontal, "Trial 2", Qt::DisplayRole);
    m_model->setHeaderData(3, Qt::Horizontal, "Trial 3", Qt::DisplayRole);
}

void SpirometerManager::start()
{
    // connect signals and slots to QProcess one time only
    //
    connect(&m_process, &QProcess::started,
        this, [this]() {
            qDebug() << "process started: " << m_process.arguments().join(" ");
        });

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &SpirometerManager::readOutput);

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

void SpirometerManager::loadSettings(const QSettings& settings)
{
    // the full spec path name including exe name
    // eg., C:/Program Files (x86)/ndd Medizintechnik/Easy on-PC/Application/EasyWarePro.exe
    //
    QString exeName = settings.value(getGroup() + "/client/exe").toString();
    QString path = settings.value(getGroup() + "/client/data").toString();
    selectRunnable(exeName);
    selectDataPath(path);
}

void SpirometerManager::saveSettings(QSettings* settings) const
{
    if(!m_runnableName.isEmpty())
    {
        settings->beginGroup(getGroup());
        settings->setValue("client/exe", m_runnableName);
        settings->endGroup();
        if(m_verbose)
            qDebug() << "wrote exe fullspec path to settings file";
    }

    if(!m_dataPath.isEmpty())
    {
        settings->beginGroup(getGroup());
        settings->setValue("client/data", m_dataPath);
        settings->endGroup();
        if(m_verbose)
            qDebug() << "wrote emr transfer directory path to settings file";
    }
}

QJsonObject SpirometerManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    if(Constants::RunMode::modeSimulate != m_mode)
    {
        QFile ofile(getEMROutXmlName());
        if(ofile.exists())
        {
            ofile.open(QIODevice::ReadOnly);
            QByteArray buffer = ofile.readAll();
            json.insert("test_output_file", QString(buffer.toBase64()));
            json.insert("test_output_file_mime_type", "xml");
        }

        if(outputPdfExists())
        {
            QString outputPdf = getOutputPdfPath();
            QFile pdfFile(outputPdf);
            if(pdfFile.open(QIODevice::ReadOnly))
            {
              QByteArray bufferPdf = pdfFile.readAll();
              json.insert("test_output_pdf_file", QString(bufferPdf.toBase64()));
              json.insert("test_output_pdf_file_mime_type", "pdf");
              pdfFile.close();
            }

        }
    }
    json.insert("test_input",QJsonObject::fromVariantMap(m_inputData));
    return json;
}

void SpirometerManager::updateModel()
{
    // Data from all measurements:
    // First list is a header and each list after is a trial
    QList<QStringList> data = m_test.toStringListList();
    qDebug() << "update model" << data.size();
    int numLists = data.count();
    int numElementsPerList = 0 < numLists ? data[0].count() : 0;

    // Set row count
    int n_row = qMax(1, numElementsPerList);
    if(n_row != m_model->rowCount())
    {
        m_model->setRowCount(n_row);
    }

    // set column count
    int n_col = qMax(1, numLists);
    if(n_col != m_model->columnCount())
    {
        m_model->setColumnCount(n_col);
    }

    for(int i = 0; i < n_col; i++)
    {
        QString colName = 0 == i ? "Data Type": QString("Trial %1").arg(i);
        m_model->setHeaderData(i, Qt::Horizontal, colName, Qt::DisplayRole);
    }

    for(int row = 0; row < numElementsPerList; row++)
    {
        for(int col = 0; col < numLists; col++)
        {
            QString dataStr = data[col][row];

            QStandardItem* item = m_model->item(row, col);
            if(nullptr == item)
            {
                item = new QStandardItem();
                m_model->setItem(row, col, item);
            }
            item->setData(dataStr, Qt::DisplayRole);
        }
    }
    emit dataChanged();
}

bool SpirometerManager::isDefined(const QString& value, const SpirometerManager::FileType &fileType) const
{
    if(value.isEmpty())
        return false;

    bool ok = false;
    if(fileType == SpirometerManager::FileType::EasyWareExe)
    {
        QFileInfo info(value);
        if(info.exists() /* && "exe" == info.completeSuffix()*/)
        {
            ok = true;
        }
    }
    else if(fileType == SpirometerManager::FileType::EMRDataPath)
    {
        if(QDir(value).exists())
        {
            ok = true;
        }
    }    
    return ok;
}

void SpirometerManager::measure()
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

void SpirometerManager::setInputData(const QVariantMap& input)
{
    m_inputData = input;
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        if(!input.contains("barcode"))
            m_inputData["barcode"] = Constants::DefaultBarcode;
        if(!input.contains("language"))
            m_inputData["language"] = "en";
        if(!input.contains("gender"))
            m_inputData["gender"] = "male";  // required
        if(!input.contains("date_of_birth"))
            m_inputData["date_of_birth"] = QDate().fromString("1994-09-25","yyy-MM-dd"); // required
        if(!input.contains("height"))
            m_inputData["height"] = 1.8; // m, required
        if(!input.contains("weight"))
            m_inputData["weight"] = 109;  // kg, optional, no decimal
        if(!input.contains("smoker"))
            m_inputData["smoker"] = false; // optional
    }
    bool ok = true;
    QMap<QString, QMetaType::Type> typeMap{
        {"barcode",QMetaType::Type::QString},
        {"language",QMetaType::Type::QString},
        {"gender",QMetaType::Type::QString},
        {"date_of_birth",QMetaType::Type::QDate},
        {"height",QMetaType::Type::Double},
        {"weight",QMetaType::Type::Double},
        {"smoker",QMetaType::Type::Bool}
    };
    foreach(auto key, m_inputKeyList)
    {
        if(!m_inputData.contains(key))
        {
            ok = false;
            qCritical() << "ERROR: invalid input data";
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
        qCritical() << "ERROR: invalid input data";
        emit message(tr("ERROR: the input data is incorrect"));
        m_inputData = QVariantMap();
    }
    else
      configureProcess();
}

void SpirometerManager::select()
{
    QFileDialog dialog;
    // which do we need to select first ?
    QString caption;    
    bool selectingRunnable = false;
    if(!isDefined(m_runnableName, SpirometerManager::FileType::EasyWareExe))
    {
        QStringList filters;
        filters << "Applications (*.exe)" << "Any files (*)";
        caption = tr("Select EasyWarePro.exe File");
        selectingRunnable = true;
        dialog.setNameFilters(filters);
        dialog.setFileMode(QFileDialog::ExistingFile);
    }
    else if(!isDefined(m_dataPath, SpirometerManager::FileType::EMRDataPath))
    {
        dialog.setFileMode(QFileDialog::FileMode::DirectoryOnly);
        caption = tr("Select EMR data transfer directory");
    }
    else
        return;

    dialog.setWindowTitle(caption);
    if(dialog.exec() == QDialog::Accepted)
    {
      QStringList files = dialog.selectedFiles();
      QString fileName = files.first();
      FileType type =
        (selectingRunnable ? FileType::EasyWareExe : FileType::EMRDataPath);
      if(isDefined(fileName,type))
      {
        if(selectingRunnable)
        {
          selectRunnable(fileName);
        }
        else
        {
          selectDataPath(fileName);
        }
      }
   }
}

void SpirometerManager::selectRunnable(const QString& exeName)
{
    if(isDefined(exeName, SpirometerManager::FileType::EasyWareExe))
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

void SpirometerManager::selectDataPath(const QString& path)
{
    if(isDefined(path, SpirometerManager::FileType::EMRDataPath))
    {
        m_dataPath = path;
        emit dataPathSelected();
        configureProcess();
    }
    else
        emit canSelectDataPath();
}

void SpirometerManager::readOutput()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        qDebug() << "manager simulate";
        m_test.simulate(m_inputData);
        if(m_test.isValid())
        {
          // emit the can write signal
          emit message(tr("Ready to save results..."));
          emit canWrite();
        }
        updateModel();
        return;
    }

    if(QProcess::NormalExit != m_process.exitStatus())
    {
      qDebug() << "ERROR: process failed to finish correctly: cannot read output";
      return;
    }
    else
      qDebug() << "process finished successfully";

    m_test.fromFile(getEMROutXmlName());
    if(m_test.isValid())
    {
      emit message(tr("Ready to save results..."));
      emit canWrite();
    }
    else
      qDebug() << "ERROR: EMR plugin produced invalid xml test results";

    updateModel();
}

void SpirometerManager::clearData()
{
    m_test.reset();
    updateModel();
}

void SpirometerManager::backupDatabases() const
{
    // Create database copy
    QString fromFile = getEWPDbName();
    QString toFile = getEWPDbCopyName();
    if(QFile::exists(fromFile))
    {
        qDebug() << "copied" << fromFile << "->" << toFile;
        // Create the copy
        QFile::copy(fromFile, toFile);
    }

    // Create database options copy
    fromFile = getEWPOptionsDbName();
    toFile = getEWPOptionsDbCopyName();
    if(QFile::exists(fromFile))
    {
        qDebug() << "copied" << fromFile << "->" << toFile;
        // Create the copy
        QFile::copy(fromFile, toFile);
    }
}

void SpirometerManager::restoreDatabases() const
{
    // Reset copied database
    QString toFile = getEWPDbName();
    QString fromFile = getEWPDbCopyName();
    if(QFile::exists(fromFile))
    {
        // Remove the current db file if it exists
        if(QFile::exists(toFile))
        {
            QFile::remove(toFile);
        }

        // Rename copy to be the current database
        QFile::rename(fromFile, toFile);
    }

    // Reset copied database options
    toFile = getEWPOptionsDbName();
    fromFile = getEWPOptionsDbCopyName();
    if(QFile::exists(fromFile))
    {
        // Remove the current db file if it exists
        if(QFile::exists(toFile))
        {
            QFile::remove(toFile);
        }

        // Rename copy to be the current database
        QFile::rename(fromFile, toFile);
    }
}

void SpirometerManager::configureProcess()
{
    if(m_inputData.isEmpty()) return;

    if(Constants::RunMode::modeSimulate == m_mode)
    {
        emit message(tr("Ready to measure..."));
        emit canMeasure();
        return;
    }

    QDir working(m_runnablePath);
    if(isDefined(m_runnableName, SpirometerManager::FileType::EasyWareExe) &&
       isDefined(m_dataPath, SpirometerManager::FileType::EMRDataPath) &&
       working.exists())
    {
        qDebug() << "OK: configuring command";

        m_process.setProgram(m_runnableName);
        m_process.setWorkingDirectory(m_runnablePath);

        removeXmlFiles();
        backupDatabases();

        // write the inputs to EMR xml
        //
        qDebug() << "creating plugin xml";
        EMRPluginWriter writer;
        writer.setInputData(m_inputData);
        QDir xmlPath(m_dataPath);
        writer.write(xmlPath.filePath("CypressIn.xml"));

        emit message(tr("Ready to measure..."));
        emit canMeasure();
    }
    else
        qDebug() << "failed to configure process";
}

void SpirometerManager::finish()
{   
    if(Constants::RunMode::modeSimulate == m_mode)
    {
      return;
    }

    if(QProcess::NotRunning != m_process.state())
    {
       m_process.kill();
    }

    restoreDatabases();
    removeXmlFiles();

    // delete pdf output file
    //
    QString pdfFilePath = getOutputPdfPath();
    if(QFile::exists(pdfFilePath))
    {
        qDebug() << "remove pdf" << pdfFilePath;
        QFile::remove(pdfFilePath);
    }

    m_test.reset();
}

void SpirometerManager::removeXmlFiles() const
{
    // delete CypressOut.xml if it exists
    //
    QString fileName = getEMROutXmlName();
    if(QFile::exists(fileName))
    {
        qDebug() << "removed" << fileName;
        QFile::remove(fileName);
    }

    // delete CypressIn.xml if it exists
    //
    fileName = getEMRInXmlName();
    if(QFile::exists(fileName))
    {
        qDebug() << "removed" << fileName;
        QFile::remove(fileName);
    }
}

QString SpirometerManager::getOutputPdfPath() const
{
    if(m_test.isValid() &&
       m_test.hasMetaData("pdf_report_path"))
    {
      return m_test.getMetaDataAsString("pdf_report_path");
    }
    else
      return QString();
}

bool SpirometerManager::outputPdfExists() const
{
    bool pdfExists = false;
    QString outPdfPath = getOutputPdfPath();
    if(!outPdfPath.isEmpty())
    {
      pdfExists = QFileInfo::exists(outPdfPath);
    }
    return pdfExists;
}
