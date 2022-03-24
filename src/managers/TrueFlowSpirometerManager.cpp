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
#include "../auxiliary/JsonSettings.h"
#include <QFileDialog>

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
            qDebug() << "wrote exe fullspec path to settings file";
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

        if(outputPdfExists()){
            QFile pdfFile(getOutputPdfPath());
            pdfFile.open(QIODevice::ReadOnly);
            QByteArray bufferPdf = pdfFile.readAll();
            json.insert("test_output_pdf_file", QString(bufferPdf.toBase64()));
            json.insert("test_output_pdf_file_mime_type", "pdf");
        }
    }
    //QJsonObject jsonInput;
    //for(auto&& x : m_inputData.toStdMap())
    //{
    //    // convert to space delimited phrases to snake_case
    //    //
    //    jsonInput.insert(QString(x.first).toLower().replace(QRegExp("[\\s]+"),"_"), QJsonValue::fromVariant(x.second));
    //}
    //json.insert("test_input", jsonInput);
    json.insert("test_input",m_inputData);
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

bool TrueFlowSpirometerManager::isDefined(const QString& value, const TrueFlowSpirometerManager::FileType &fileType) const
{
    if (value.isEmpty())
    {
        return false;
    }

    bool ok = false;
    if (fileType == TrueFlowSpirometerManager::FileType::EasyWareExe) {
        QFileInfo info(value);
        if (info.exists() && "exe" == info.completeSuffix())
        {
            ok = true;
        }
    }
    else if (fileType == TrueFlowSpirometerManager::FileType::TransferDir) {
        if (QDir(value).exists()) {
            ok = true;
        }
    }
    
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

void TrueFlowSpirometerManager::setInputData(const QJsonObject& input)
{
    m_inputData = input;
    if (Constants::RunMode::modeSimulate == m_mode)
    {
        if (!input.contains("barcode"))
            m_inputData["barcode"] = Constants::DefaultBarcode;
        if (!input.contains("language"))
            m_inputData["language"] = "en";
        if (!input.contains("gender"))
            m_inputData["gender"] = "Male";
        if (!input.contains("date_of_birth"))
            m_inputData["date_of_birth"] = "1950-12-25";
        if (!input.contains("height"))
            m_inputData["height"] = 1.5;
        if (!input.contains("weight"))
            m_inputData["weight"] = 80;
        if (!input.contains("smoker"))
            m_inputData["smoker"] = false;
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
        if (!m_inputData.contains(key))
        {
            ok = false;
            if (m_verbose)
                qDebug() << "ERROR: missing expected input " << key;
            break;
        }
        const QVariant value = m_inputData[key].toVariant();
        bool valueOk = true;
        QMetaType::Type type;
        if (typeMap.contains(key))
        {
            type = typeMap[key];
            valueOk = value.canConvert(type);
        }
        if (!valueOk)
        {
            ok = false;
            if (m_verbose)
                qDebug() << "ERROR: invalid input" << key << value.toString() << QMetaType::typeName(type);
            break;
        }
    }
    if (!ok)
    {
        if (m_verbose)
            qDebug() << "ERROR: invalid input data";

        emit message(tr("ERROR: the input data is incorrect"));
        m_inputData = QJsonObject();
    }
    else
    {
        // Get participat info from inputs
        QString gender = m_inputData["gender"].toString();
        QDate birthDate = m_inputData["date_of_birth"].toVariant().toDate();
        double height = m_inputData["height"].toDouble();
        double weight = m_inputData["weight"].toDouble();
        bool smoker = m_inputData["smoker"].toBool();

        // load participant info
        m_onyxInXml.setParticipantInfo(gender, birthDate, height, weight, smoker);
    }
}

void TrueFlowSpirometerManager::select()
{
    // which do we need to select first ?
    QString caption;
    QStringList filters;
    bool selectingRunnable = false;
    if (!isDefined(m_runnableFullPath, TrueFlowSpirometerManager::FileType::EasyWareExe))
    {
        filters << "Applications (*.exe)" << "Any files (*)";
        caption = tr("Select EasyWarePro.exe File");
        selectingRunnable = true;
    }
    else if (!isDefined(m_emrTransferDir, TrueFlowSpirometerManager::FileType::TransferDir))
    {
        caption = tr("Select EMR Transfer Directory");
    }
    else
        return;

    QFileDialog dialog;
    dialog.setNameFilters(filters);
    dialog.setFileMode(selectingRunnable ? QFileDialog::ExistingFile: QFileDialog::Directory);
    dialog.setWindowTitle(caption);
    if (dialog.exec() == QDialog::Accepted)
    {
        if (selectingRunnable) {
            QStringList files = dialog.selectedFiles();
            QString fileName = files.first();
            if (isDefined(fileName, FileType::EasyWareExe)) {
                selectRunnable(fileName);
            }
        }
        else {
            QString dirName = dialog.directory().absolutePath();
            if (isDefined(dirName, FileType::TransferDir)) {
                selectEmrTransferDir(dirName);
            }
        }
    }
}

void TrueFlowSpirometerManager::selectRunnable(const QString& runnableFullPath)
{
    if (isDefined(runnableFullPath, TrueFlowSpirometerManager::FileType::EasyWareExe))
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
    if (isDefined(emrTransferDir, TrueFlowSpirometerManager::FileType::TransferDir))
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
    QString pdfFilePath = getOutputPdfPath();
    if (QFile::exists(pdfFilePath)) {
        QFile::remove(pdfFilePath);
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
    bool initialized = isDefined(m_runnableFullPath, TrueFlowSpirometerManager::FileType::EasyWareExe)
        && isDefined(m_runnableDir, TrueFlowSpirometerManager::FileType::TransferDir);
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
    if (isDefined(m_runnableFullPath, TrueFlowSpirometerManager::FileType::EasyWareExe) &&
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

QString TrueFlowSpirometerManager::getOutputPdfPath() const
{
    bool hasPdfPath = m_test.getMetaData().getAttributes().contains("pdfPath");
    if (hasPdfPath) {
        return m_test.getMetaData().getAttribute("pdfPath").toString();
    }
    return QString();
}

bool TrueFlowSpirometerManager::outputPdfExists() const{
    bool pdfExists = false;
    QString outPdfPath = getOutputPdfPath();
    if (outPdfPath != QString()) {
        QFile pdfFile(outPdfPath);
        pdfExists = pdfFile.exists();
    }
    return pdfExists;
}