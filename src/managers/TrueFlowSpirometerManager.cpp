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
    QString transferDir = settings.value(getGroup() + "/client/transferDir").toString();
    QString transferInFilename = settings.value(getGroup() + "/client/transferInFilename").toString();
    QString transferOutFilename = settings.value(getGroup() + "/client/transferOutFilename").toString();
    selectRunnable(exeName);
}

void TrueFlowSpirometerManager::saveSettings(QSettings* settings) const
{
}

QJsonObject TrueFlowSpirometerManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    if(CypressConstants::RunMode::Simulate == m_mode)
    {
        // Simulate mode code
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
    if(CypressConstants::RunMode::Simulate == m_mode)
    {
        readOutput();
        return;
    }

    clearData();
    // launch the process
    qDebug() << "starting process from measure";
    
    

    QString easyOnPcDirPath = "C:/ProgramData/ndd/Easy on-PC";
    ResetEasyOnPcFiles(easyOnPcDirPath);
    m_onyxInXml.write(easyOnPcDirPath);
    LaunchEasyOnPc();
    readOutput();
}

void TrueFlowSpirometerManager::setInputData(const QMap<QString, QVariant>& input)
{
    if(CypressConstants::RunMode::Simulate == m_mode)
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

        emit runnableSelected();
        configureProcess();
    }
    else
        emit canSelectRunnable();
}

void TrueFlowSpirometerManager::readOutput()
{
    if(CypressConstants::RunMode::Simulate == m_mode)
    {
        // TODO: Implement simulate mode
        return;
    }

    // Read the output for non simulate mode
    bool loaded = m_test.loadData();
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

void TrueFlowSpirometerManager::LaunchEasyOnPc() 
{
    m_process.start();
    bool launched = m_process.waitForFinished(100000000);
    m_process.close();
    qDebug() << "Easy on pc launched = " << launched << endl;
}

void TrueFlowSpirometerManager::ResetEasyOnPcFiles(const QString& dirPath) const
{
    // Delete OnyxOut.xml if it exists
    QString onyxOutFilePath = QString("%1/OnyxOut.xml").arg(dirPath);
    if (QFile::exists(onyxOutFilePath)) {
        QFile::remove(onyxOutFilePath);
    }

    // Delete OnyxIn.xml if it exists
    QString onyxInFilePath = QString("%1/OnyxIn.xml").arg(dirPath);
    if (QFile::exists(onyxInFilePath)) {
        QFile::remove(onyxInFilePath);
    }

    // TODO: Remove patient entries from EasyOnPc
    // NOTE: Not sure where this data is stored
}

void TrueFlowSpirometerManager::configureProcess()
{
    if (CypressConstants::RunMode::Simulate == m_mode)
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

        // TODO: Check that a correct version of java is installed
        QString command = QString("\"%1\"").arg(m_runnableFullPath);
        //command = QString("\"C:\\Program Files (x86)\\ndd Medizintechnik\\Easy on-PC\\Application\\EasyWarePro.exe\"");
        QStringList arguments;
        //arguments << "-jar"
        //    << m_runnableFullPath
        //    << getInputDataValue("barcode").toString();

        m_process.setProgram(command);
        //m_process.setArguments(arguments);
        m_process.setWorkingDirectory(working.absolutePath());

        //qDebug() << "process config args: " << m_process.arguments().join(" ");
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
}
