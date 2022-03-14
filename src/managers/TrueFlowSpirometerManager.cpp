#include "TrueFlowSpirometerManager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>
#include <QStandardItemModel>
#include <QProcess>
#include <QFile>

#include "../prototype/TrueFlowSpirometer/OnyxOutXml.h"

TrueFlowSpirometerManager::TrueFlowSpirometerManager(QObject* parent) : ManagerBase(parent)
{
    setGroup("trueflowspirometer");

    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode";
    m_inputKeyList << "language";
    m_inputKeyList << "gender";
    m_inputKeyList << "date_of_birth";
    m_inputKeyList << "height";
    m_inputKeyList << "weight";
    //m_inputKeyList << "ethnicity";
    m_inputKeyList << "smoker";
    //m_inputKeyList << "asthma";
    //m_inputKeyList << "copd";
}

TrueFlowSpirometerManager::~TrueFlowSpirometerManager()
{
}

void TrueFlowSpirometerManager::start()
{
    emit dataChanged();
}

void TrueFlowSpirometerManager::loadSettings(const QSettings& settings)
{
    // the full spec path name including exe name
    // eg., C:/Program Files (x86)/ndd Medizintechnik/Easy on-PC/Application/EasyWarePro.exe
    //
    QString exeName = settings.value(getGroup() + "/client/exe").toString();
    QString emrTransferDir = settings.value(getGroup() + "/client/transferDir").toString();
    selectRunnable(exeName);
    selectEmrTransferDir(emrTransferDir);
}

void TrueFlowSpirometerManager::saveSettings(QSettings* settings) const
{
    if (!m_runnableFullPath.isEmpty())
    {
        settings->beginGroup(getGroup());
        settings->setValue("client/exe", m_runnableFullPath);
        settings->endGroup();
        if (m_verbose)
            qDebug() << "wrote jar fullspec path to settings file";
    }

    if (!m_emrTransferDir.isEmpty())
    {
        settings->beginGroup(getGroup());
        settings->setValue("client/transferDir", m_emrTransferDir);
        settings->endGroup();
        if (m_verbose)
            qDebug() << "wrote emr transfer directory path to settings file";
    }

    resetEmrTransferFiles();
}

QJsonObject TrueFlowSpirometerManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    if (Constants::RunMode::modeSimulate != m_mode)
    {
        QFile ofile(getTransferOutFilePath());
        if (ofile.exists()) {
            ofile.open(QIODevice::ReadOnly);
            QByteArray buffer = ofile.readAll();
            json.insert("test_output_file", QString(buffer.toBase64()));
            json.insert("test_output_file_mime_type", "xml");
        }
        

        QFile pdfFile(getOutPdfFilePath());
        if (pdfFile.exists()) {
            pdfFile.open(QIODevice::ReadOnly);
            QByteArray bufferPdf = pdfFile.readAll();
            json.insert("test_output_pdf_file", QString(bufferPdf.toBase64()));
            json.insert("test_output_pdf_file_mime_type", "pdf");
        }
    }
    QJsonObject jsonInput;
    for(auto&& x : m_inputData.toStdMap())
    {
        // convert to space delimited phrases to snake_case
        //
        jsonInput.insert(QString(x.first).toLower().replace(QRegExp("[\\s]+"),"_"), QJsonValue::fromVariant(x.second));
    }
    json.insert("test_input",jsonInput);
    return json;
}

void TrueFlowSpirometerManager::buildModel(QStandardItemModel* model) const
{
    // add measurements one row of two columns at a time
    //
    int n_total = m_test.getNumberOfMeasurements();
    int n_row = qMax(1, n_total / 2);
    if (n_row != model->rowCount())
    {
        model->setRowCount(n_row);
    }
    int row_left = 0;
    int row_right = 0;
    for (int i = 0; i < n_total; i++)
    {
        TrueFlowSpirometerMeasurement measurement = m_test.getMeasurement(i);
        QString measurementStr = measurement.isValid() ? measurement.toString() : "NA";

        int col = i % 2;
        int* row = col == 0 ? &row_left : &row_right;
        QStandardItem* item = model->item(*row, col);
        if (nullptr == item)
        {
            item = new QStandardItem();
            model->setItem(*row, col, item);
        }
        item->setData(measurementStr, Qt::DisplayRole);
        (*row)++;
    }
}

bool TrueFlowSpirometerManager::isDefined(const QString& runnableFullPath) const
{
    bool ok = false;
    if (!runnableFullPath.isEmpty())
    {
        QFileInfo info(runnableFullPath);
        if (info.exists() && "exe" == info.completeSuffix())
        {
            ok = true;
        }
        else
            qDebug() << "ERROR: info does not exist for file " << runnableFullPath;
    }
    else
        qDebug() << "ERROR: isDefined check on empty string";
    return ok;
}

void TrueFlowSpirometerManager::measure()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        readOutput();
        return;
    }

    clearData();
    // launch the process
    qDebug() << "starting process from measure";
    
    resetEmrTransferFiles();
    m_onyxInXml.write(getTransferInFilePath());
    launchEasyOnPc();
    readOutput();
}

void TrueFlowSpirometerManager::setInputData(const QMap<QString, QVariant>& input)
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        m_inputData["barcode"] = "00000000";
        m_inputData["language"] = "english";
        m_inputData["gender"] = "Male";
        m_inputData["date_of_birth"] = "1950-12-25";
        m_inputData["height"] = 1.8;
        m_inputData["weight"] = 100;
        m_inputData["smoker"] = false;
        return;
    }
    bool ok = true;
    m_inputData = input;
    for(auto&& x : m_inputKeyList)
    {
        if(!input.contains(x))
        {
            ok = false;
            break;
        }
    }
    if(!ok)
        m_inputData.clear();
    else
    {
        // Get participat info from inputs
        QString gender = m_inputData["gender"].toString();
        QDate birthDate = m_inputData["date_of_birth"].toDate();
        double height = m_inputData["height"].toDouble();
        double weight = m_inputData["weight"].toDouble();
        bool smoker = m_inputData["smoker"].toBool();

        // load participant info
        m_onyxInXml.setParticipantInfo(gender, birthDate, height, weight, smoker);
    }
}

void TrueFlowSpirometerManager::selectRunnable(const QString& runnableFullPath)
{
    if (isDefined(runnableFullPath))
    {
        QFileInfo info(runnableFullPath);
        m_runnableFullPath = runnableFullPath;
        m_runnableDir = info.absolutePath();

        if(inputDataInitialized()){
            configureProcess();
        }
    }
    else
        emit canSelectRunnable();
}

void TrueFlowSpirometerManager::selectEmrTransferDir(const QString& emrTransferDir)
{
    if (emrTransferDir.isEmpty() == false && QDir(emrTransferDir).exists())
    {
        m_emrTransferDir = emrTransferDir;

        if (inputDataInitialized()) {
            configureProcess();
        }
    }
    else
        emit canSelectEmrTransferDir();
}

void TrueFlowSpirometerManager::readOutput()
{
    if(Constants::RunMode::modeSimulate == m_mode)
    {
        // TODO: Implement simulate mode
        return;
    }

    // Read the output for non simulate mode
    bool loaded = m_test.loadData(getTransferOutFilePath());
    if (loaded) {
        emit canWrite();
    }
    else {
        // Todo: do something here
    }
}

void TrueFlowSpirometerManager::clearData()
{
    m_test.reset();
    emit dataChanged();
}

void TrueFlowSpirometerManager::launchEasyOnPc() 
{
    createDatabaseCopies();

    m_process.start();
    bool launched = m_process.waitForFinished(100000000);
    m_process.close();
    qDebug() << "Easy on pc launched = " << launched << endl;
}

void TrueFlowSpirometerManager::resetEmrTransferFiles() const
{
    // Delete OnyxOut.xml if it exists
    if (QFile::exists(getTransferOutFilePath())) {
        QFile::remove(getTransferOutFilePath());
    }

    // Delete OnyxIn.xml if it exists
    if (QFile::exists(getTransferInFilePath())) {
        QFile::remove(getTransferInFilePath());
    }

    // Reset copied database
    QString dbPath = getDbPath();
    QString dbCopyPath = getDbCopyPath();
    if (QFile::exists(dbCopyPath)) {
        // Remove the current db file if it exists
        if (QFile::exists(dbPath)) {
            QFile::remove(dbPath);
        }

        // Rename copy to be the current database
        QFile::rename(dbCopyPath, dbPath);
    }

    // Reset copied database options
    QString dbOptionsPath = getDbOptionsPath();
    QString dbOptionsCopyPath = getDbOptionsCopyPath();
    if (QFile::exists(dbOptionsCopyPath)) {
        // Remove the current db file if it exists
        if (QFile::exists(dbOptionsPath)) {
            QFile::remove(dbOptionsPath);
        }

        // Rename copy to be the current database
        QFile::rename(dbOptionsCopyPath, dbOptionsPath);
    }

    // Delete pdf output file
    QString pdfFilePatyh = getOutPdfFilePath();
    if (QFile::exists(pdfFilePatyh)) {
        QFile::remove(pdfFilePatyh);
    }
}

void TrueFlowSpirometerManager::createDatabaseCopies() const
{
    // Create database copy
    QString dbPath = getDbPath();
    QString dbCopyPath = getDbCopyPath();
    if (QFile::exists(dbPath)) {
        // Create the copy
        QFile::copy(dbPath, dbCopyPath);
    }

    // Create database options copy
    QString dbOptionsPath = getDbOptionsPath();
    QString dbOptionsCopyPath = getDbOptionsCopyPath();
    if (QFile::exists(dbOptionsPath)) {
        // Create the copy
        QFile::copy(dbOptionsPath, dbOptionsCopyPath);
    }
}

bool TrueFlowSpirometerManager::inputDataInitialized() const
{
    bool initialized =
        m_runnableDir.isEmpty() == false
        && QDir(m_runnableDir).exists()
        && isDefined(m_runnableFullPath)
        && m_emrTransferDir.isEmpty() == false;
    return initialized;
}

void TrueFlowSpirometerManager::configureProcess()
{
    if (Constants::RunMode::modeSimulate == m_mode)
    {
        if (!m_inputData.isEmpty())
        {
            emit message(tr("Ready to measure..."));
            emit canMeasure();
        }
        return;
    }

    QDir working(m_runnableDir);
    if (isDefined(m_runnableFullPath) &&
        working.exists())
    {
        qDebug() << "OK: configuring command";

        QString command = QString("\"%1\"").arg(m_runnableFullPath);
        m_process.setProgram(command);
        m_process.setWorkingDirectory(working.absolutePath());
        qDebug() << "process working dir: " << working.absolutePath();

        emit message(tr("Ready to measure..."));
        emit canMeasure();
    }
    else
        qDebug() << "failed to configure process";
}

void TrueFlowSpirometerManager::finish()
{
    m_test.reset();
    resetEmrTransferFiles();
}
