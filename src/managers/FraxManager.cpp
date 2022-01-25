#include "FraxManager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QSettings>
#include <QStandardItemModel>

FraxManager::FraxManager(QObject* parent):
    ManagerBase(parent)
{
    setGroup("frax");

    // all managers must check for barcode and language input values
    //
    m_inputKeyList << "barcode";
    m_inputKeyList << "language";

    // key/value pairs needed to build input.txt
    //
    m_inputKeyList << "type";
    m_inputKeyList << "country_code";
    m_inputKeyList << "age";
    m_inputKeyList << "sex";
    m_inputKeyList << "bmi";
    m_inputKeyList << "previous_fracture";
    m_inputKeyList << "parent_hip_fracture";
    m_inputKeyList << "current_smoker";
    m_inputKeyList << "gluccocorticoid";
    m_inputKeyList << "rheumatoid_arthritis";
    m_inputKeyList << "secondary_osteoporosis";
    m_inputKeyList << "alcohol";
    m_inputKeyList << "femoral_neck_bmd";
}

void FraxManager::start()
{
    configureProcess();
    emit dataChanged();
}

void FraxManager::buildModel(QStandardItemModel *model) const
{
    // add the four probability measurements
    //
    for(int i = 0; i < m_test.getNumberOfMeasurements(); i++)
    {
        QStandardItem* item = model->item(i, 0);
        if (Q_NULLPTR == item)
        {
            item = new QStandardItem();
            model->setItem(i, 0, item);
        }
        item->setData(m_test.getMeasurement(i).toString(), Qt::DisplayRole);
    }
}

void FraxManager::loadSettings(const QSettings& settings)
{
    // the full spec path name including exe name
    // eg., ../frax_module/blackbox.exe
    //
    QString exeName = settings.value(getGroup() + "/client/exe").toString();
    selectRunnable(exeName);
}

void FraxManager::saveSettings(QSettings* settings) const
{
    if (!m_runnableName.isEmpty())
    {
        settings->beginGroup(getGroup());
        settings->setValue("client/exe", m_runnableName);
        settings->endGroup();
        if (m_verbose)
            qDebug() << "wrote exe fullspec path to settings file";
    }
}

QJsonObject FraxManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    if("simulate" != m_mode)
    {
      QFile ofile(m_outputFile);
      ofile.open(QIODevice::ReadOnly);
      QByteArray buffer = ofile.readAll();
      json.insert("test_output_file",QString(buffer.toBase64()));
      json.insert("test_output_file_mime_type","txt");
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

bool FraxManager::isDefined(const QString &exeName) const
{
    if("simulate" == m_mode)
    {
       return true;
    }
    bool ok = false;
    if(!exeName.isEmpty())
    {
        QFileInfo info(exeName);
        if(info.exists() && info.isExecutable())
        {
            ok = true;
        }
    }
    return ok;
}

void FraxManager::selectRunnable(const QString &runnableName)
{
    if(isDefined(runnableName))
    {
        QFileInfo info(runnableName);
        m_runnableName = runnableName;
        m_runnablePath = info.absolutePath();
        m_outputFile = QDir(m_runnablePath).filePath("output.txt");
        m_inputFile =  QDir(m_runnablePath).filePath("input.txt");
        m_temporaryFile = QDir(m_runnablePath).filePath("input_ORIG.txt");

        configureProcess();
    }
    else
        emit canSelectRunnable();
}

void FraxManager::measure()
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

void FraxManager::setInputData(const QMap<QString, QVariant> &input)
{
    if("simulate" == m_mode)
    {
        m_inputData["barcode"] = "00000000";
        m_inputData["language"] = "english";

        m_inputData["type"] = "t";
        m_inputData["country_code"] = 19;
        m_inputData["age"] = 84.19;
        m_inputData["sex"] = 0;
        m_inputData["bmi"] = 24.07;
        m_inputData["previous_fracture"] = 0;
        m_inputData["parent_hip_fracture"] = 0;
        m_inputData["current_smoker"] = 0;
        m_inputData["gluccocorticoid"] = 0;
        m_inputData["rheumatoid_arthritis"] = 0;
        m_inputData["secondary osteoporosis"] = 0;
        m_inputData["alcohol"] = 0;
        m_inputData["femoral_neck_bmd"] = -1.1;
        return;
    }
    bool ok = true;
    m_inputData = input;
    for(auto&& x : m_inputKeyList)
    {
        if(!input.contains(x))
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

void FraxManager::readOutput()
{
    if("simulate" == m_mode)
    {
        qDebug() << "simulating read out";

        FraxMeasurement m;
        m.setCharacteristic("type","osteoporotic_fracture");
        m.setCharacteristic("probability", 1.0);
        m.setCharacteristic("units","%");
        m_test.addMeasurement(m);
        m.setCharacteristic("type","hip_fracture");
        m_test.addMeasurement(m);
        m.setCharacteristic("type","osteoporotic_fracture_bmd");
        m_test.addMeasurement(m);
        m.setCharacteristic("type","hip_fracture_bmd");
        m_test.addMeasurement(m);

        for(auto&& x : m_inputData.toStdMap())
        {
          m_test.addMetaDataCharacteristic(x.first,x.second);
        }
        emit message(tr("Ready to save results..."));
        emit canWrite();
        emit dataChanged();

        return;
    }

    if (QProcess::NormalExit != m_process.exitStatus())
    {
        qDebug() << "ERROR: process failed to finish correctly: cannot read output";
        return;
    }
    else
        qDebug() << "process finished successfully";

    if(QFileInfo::exists(m_outputFile))
    {
        qDebug() << "found output txt file " << m_outputFile;
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
        qDebug() << "ERROR: no output.txt file found";
}

void FraxManager::configureProcess()
{
    if("simulate" == m_mode &&
       !m_inputData.isEmpty())
    {
        emit message(tr("Ready to measure..."));
        emit canMeasure();
        return;
    }

    // blackbox.exe and input.txt file are present
    //
    QFileInfo info(m_runnableName);
    QDir working(m_runnablePath);
    if (info.exists() && info.isExecutable() &&
        working.exists() && QFileInfo::exists(m_inputFile) &&
       !m_inputData.isEmpty())
    {
        qDebug() << "OK: configuring command";

        m_process.setProcessChannelMode(QProcess::ForwardedChannels);
        m_process.setProgram(m_runnableName);
        m_process.setWorkingDirectory(m_runnablePath);

        qDebug() << "process working dir: " << m_runnablePath;

        // backup the original intput.txt
        //
        if(!QFileInfo::exists(m_temporaryFile))
        {
          if(QFile::copy(m_inputFile, m_temporaryFile))
              qDebug() << "wrote backup of"<< m_inputFile << "to" << m_temporaryFile;
          else
              qDebug() << "ERROR: failed to backup default " << m_inputFile;
        }
        // generate the input.txt file content
        // exclude the interview barcode and language
        //
        QStringList list;
        for(auto&& x : m_inputKeyList)
        {
            if("barcode"==x) continue;
            if("language"==x) continue;
            if(m_inputData.contains(x))
              list << m_inputData[x].toString();
        }
        QString line = list.join(",");
        QFile ofile(m_inputFile);
        if(ofile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&ofile);
            stream << line << Qt::endl;
            ofile.close();
            qDebug() << "populated the input.txt file " << m_inputFile;
            qDebug() << "content should be " << line;
        }
        else
        {
            qDebug() << "ERROR: failed writing to " << m_inputFile;
            return;
        }

        connect(&m_process, &QProcess::started,
          this, [this]() {
              qDebug() << "process started: " << m_process.arguments().join(" ");
          });

        connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          this, &FraxManager::readOutput);

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

        emit message(tr("Ready to measure..."));
        emit canMeasure();
    }
    else
        qDebug() << "failed to configure process";
}

void FraxManager::clearData()
{
    m_test.reset();
    emit dataChanged();
}

void FraxManager::finish()
{
    m_test.reset();
    if(QProcess::NotRunning != m_process.state())
    {
        m_process.close();
    }

    // remove blackbox.exe generated output.txt file
    //
    if(!m_outputFile.isEmpty() && QFileInfo::exists(m_outputFile))
    {
        qDebug() << "removing output file " << m_outputFile;
        QFile ofile(m_outputFile);
        ofile.remove();
        m_outputFile.clear();
    }

    // remove the default input.txt file
    //
    if(!m_temporaryFile.isEmpty() && QFileInfo::exists(m_temporaryFile))
    {
        // remove the inputfile first
        QFile ifile(m_inputFile);
        ifile.remove();
        QFile::copy(m_temporaryFile, m_inputFile);
        qDebug() << "restored backup from " << m_temporaryFile;
        QFile tempFile(m_temporaryFile);
        tempFile.remove();
        m_temporaryFile.clear();
    }
}
