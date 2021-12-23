#include "CDTTManager.h"

CDTTManager::CDTTManager(QObject* parent) : ManagerBase(parent)
{
}

void CDTTManager::loadSettings(const QSettings& settings)
{
    // the full spec path name including jar name
    // eg., C:\Users\clsa\Documents\CDTT-2018-07-22\CDTTstereo.jar
    //
    QString jarFullPath = settings.value("cdtt/client/jar").toString();
    qDebug() << "loading settings with jar " << jarFullPath;
    setJarFullPath(jarFullPath);
}

void CDTTManager::saveSettings(QSettings* settings) const
{
    if (!m_jarFullPath.isEmpty())
    {
        settings->beginGroup("cdtt");
        settings->setValue("client/jar", m_jarFullPath);
        settings->endGroup();
        if (m_verbose)
            qDebug() << "wrote jar fullspec path to settings file";
    }
}

QJsonObject CDTTManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    if ("simulate" != m_mode)
    {
        QFile ofile(m_outputFile);
        ofile.open(QIODevice::ReadOnly);
        QByteArray buffer = ofile.readAll();
        json.insert("test_output_file", QString(buffer.toBase64()));
        json.insert("test_output_file_mime_type", "csv");
    }
    return json;
}

void CDTTManager::buildModel(QStandardItemModel* model) const
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
        CDTTMeasurement measurement = m_test.getMeasurement(i);
        QString measurementStr = measurement.isValid() ? measurement.toString() : "NA";

        int col = i%2;
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

bool CDTTManager::isDefined(const QString& jarFullPath) const
{
    bool ok = false;
    if (!jarFullPath.isEmpty())
    {
        QFileInfo info(jarFullPath);
        if (info.exists() && "jar" == info.completeSuffix())
        {
            QString path = info.absolutePath();

            qDebug() << "path to " << jarFullPath << " is " << path;
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
            qDebug() << "ERROR: info does not exist for file " << jarFullPath;
    }
    else
        qDebug() << "ERROR: isDefined check on empty string";
    return ok;
}

void CDTTManager::setJarFullPath(const QString& jarFullPath)
{
    if (isDefined(jarFullPath))
    {
        QFileInfo info(jarFullPath);
        m_jarFullPath = jarFullPath;
        m_jarDirPath = info.absolutePath();
        QDir dir = QDir::cleanPath(m_jarDirPath + QDir::separator() + "applicationFiles" + QDir::separator() + "Results");
        m_outputPath = dir.path();

        configureProcess();
    }
}

void CDTTManager::measure()
{
    if ("simulate" == m_mode)
    {
        readOutput();
        return;
    }

    clearData();
    // launch the process
    qDebug() << "starting process from measure";
    m_process.start();
}

void CDTTManager::clean()
{
    if (!m_outputFile.isEmpty() && QFileInfo::exists(m_outputFile))
    {
        QFile ofile(m_outputFile);
        ofile.remove();
    }
}

void CDTTManager::setInputData(const QMap<QString, QVariant>& input)
{
    if ("simulate" == m_mode)
    {
        m_inputData["barcode"] = 12345678;
        return;
    }
    bool ok = true;
    for (auto&& x : m_inputKeyList)
    {
        if (!input.contains(x))
        {
            ok = false;
            break;
        }
        else
            m_inputData[x] = input[x];
    }
    if (!ok)
        m_inputData.clear();
    else
        configureProcess();
}

void CDTTManager::readOutput()
{
    if ("simulate" == m_mode) {
        // TODO: Implement simulate mode
        return;
    }

    if (QProcess::NormalExit != m_process.exitStatus()) {
        qDebug() << "ERROR: process failed to finish correctly: cannot read output";
        return;
    }
    else {
        qDebug() << "process finished successfully";
    }

    QDir dir(m_outputPath);
    bool found = false;
    QString fileName = QString("Results-%0.xlsx").arg(m_inputData["barcode"].toString());
    for (auto&& x : dir.entryList())
    {
        if (x == fileName)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        qDebug() << "found output xlsx file " << fileName;
        QString filePath = m_outputPath + QDir::separator() + fileName;
        qDebug() << "found output xlsx file path " << filePath;
        m_test.fromFile(filePath);
        m_outputFile.clear();
        if (m_test.isValid())
        {
            emit canWrite();
            m_outputFile = filePath;
        }
        else
            qDebug() << "ERROR: input from file produced invalid test results";

        emit dataChanged();
    }
    else
        qDebug() << "ERROR: no output csv file found";
}

void CDTTManager::clearData()
{
    m_test.reset();
    m_outputFile.clear();
    emit dataChanged();
}

void CDTTManager::finish()
{
    m_test.reset();
    if(QProcess::NotRunning != m_process.state())
    {
        m_process.close();
    }
}

void CDTTManager::configureProcess()
{
    if ("simulate" == m_mode)
    {
        emit canMeasure();
        return;
    }
    // the exe is present
    QFileInfo info(m_jarFullPath);
    QDir working(m_jarDirPath);
    QDir out(m_outputPath);
    if (info.exists() && "jar" == info.completeSuffix() &&
        working.exists() && out.exists())
    {
        qDebug() << "OK: configuring command";

        // TODO: Check that a correct version of java is installed
        QString command = "java";
        QStringList arguments;
        arguments << "-jar"
            //<< "CDTTstereo.jar"
            << info.fileName()
            << m_inputData.value("barcode").toString();

        m_process.setProcessChannelMode(QProcess::ForwardedChannels);
        m_process.setProgram(command);
        m_process.setArguments(arguments);
        m_process.setWorkingDirectory(m_jarDirPath);

        qDebug() << "process config args: " << m_process.arguments().join(" ");
        qDebug() << "process working dir: " << m_jarDirPath;

        connect(&m_process, &QProcess::started,
            this, [this]() {
                qDebug() << "process started: " << m_process.arguments().join(" ");
            });

        connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CDTTManager::readOutput);

        connect(&m_process, &QProcess::errorOccurred,
            this, [](QProcess::ProcessError error)
            {
                QStringList s = QVariant::fromValue(error).toString().split(QRegExp("(?=[A-Z])"), QString::SkipEmptyParts);
                qDebug() << "ERROR: process error occured: " << s.join(" ").toLower();
            });

        connect(&m_process, &QProcess::stateChanged,
            this, [](QProcess::ProcessState state) {
                QStringList s = QVariant::fromValue(state).toString().split(QRegExp("(?=[A-Z])"), QString::SkipEmptyParts);
                qDebug() << "process state: " << s.join(" ").toLower();

            });

        emit canMeasure();
    }
    else
        qDebug() << "failed to configure process";
}
